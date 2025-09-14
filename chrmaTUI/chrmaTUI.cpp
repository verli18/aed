#include "chrmaTUI.hpp"

struct termios originalTermios;

void disableRawMode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
}
void enableRawMode(){
    tcgetattr(STDIN_FILENO, &originalTermios);
    atexit(disableRawMode);
    struct termios raw = originalTermios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~OPOST;
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);    
}

void getTerminalSize(int& rows, int& cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        // Fallback values
        rows = 24;
        cols = 80;
    } else {
        rows = ws.ws_row;
        cols = ws.ws_col;
    }
}

void TUImanager::clearScreen(color col) {
    // Move cursor to home position and clear the internal buffer to printable spaces
    // using the provided background color. Then force an immediate render so the
    // visible terminal is updated right away.
    std::cout << "\x1b[H";
    color fg = {0, 0, 0, 255};
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            // Use a printable space so render() emits a colored cell instead of skipping it
            screenBuffer[y][x] = {' ', fg, col, std::string()};
        }
    }

    // Force an immediate repaint so the cleared buffer is written to the terminal now.
    //render();
}

pressedKey mapCharToKey(char c){
    switch (c) {
        case 'w': return UP;
        case 's': return DOWN;
        case 'a': return LEFT;
        case 'd': return RIGHT;
        case 'q': return Q;
        case 127: return BACKSPACE; // Backspace/Delete
        case 13: return ENTER;     // Enter
    }
    return UNKNOWN;
}

bool TUImanager::pollInput() {
    char c = '\0';
    int nread = read(STDIN_FILENO, &c, 1);

    if (nread == -1 && errno != EAGAIN) return true; // Handle read error

    if (nread > 0) {
        pressedKey key = UNKNOWN;

        if (c == '\x1b') { // Escape sequence
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) { /* no-op */ }
            else if (read(STDIN_FILENO, &seq[1], 1) != 1) { /* no-op */ }
            else if (seq[0] == '[') {
                switch (seq[1]) {
                    case 'A': key = UP; break;
                    case 'B': key = DOWN; break;
                    case 'C': key = RIGHT; break;
                    case 'D': key = LEFT; break;
                }
            }
        } else {
            key = mapCharToKey(c);
        }

    if (key == Q && userState == NAVIGATING) return true;

        container* current = containerID;
        element* focusedElem = current ? current->getFocused() : nullptr;

        if (!focusedElem) return false;

        if (userState == CAPTURE) {
            focusedElem->onInteract(key, c, userState, *this);
            if (userState != CAPTURE) {
                // Defer the capture-end callback to end of frame so user drawings persist
                element* endedElem = focusedElem;
                enqueueEndOfFrame([endedElem](TUImanager& tui){ endedElem->notifyCaptureEnd(tui); });
            }
        } else if (userState == NAVIGATING) {
            if (key == UP || key == DOWN) {
                if (current) current->navigate(key);
            } else if (key == LEFT && current && current->getLeftPointer()) {
                current->getFocused()->notifyHover(*this, false); // Unhover old
                containerID = current->getLeftPointer();
                containerID->tui = this;
                containerID->focusedIndex = 0;
                if(!containerID->elements.empty()) containerID->getFocused()->notifyHover(*this, true); // hover new
            } else if (key == RIGHT && current && current->getRightPointer()) {
                current->getFocused()->notifyHover(*this, false); // Unhover old
                containerID = current->getRightPointer();
                containerID->tui = this;
                containerID->focusedIndex = 0;
                if(!containerID->elements.empty()) containerID->getFocused()->notifyHover(*this, true); // hover new
            } else if (key == ENTER) {
                if (focusedElem->capturesInput()) {
                    userState = CAPTURE;
                } else {
                    focusedElem->onInteract(key, c, userState, *this);
                }
            }
        }
    }
    return false;
}

bool TUImanager::windowShouldClose() {
    return pollInput();
}

void TUImanager::render() {
    // Disable line wrap to avoid auto-wrapping the last column into the next line
    std::cout << "\x1b[?7l";
    for (int y = 0; y < rows; ++y) {
        // Move cursor to start of the current line (1-based row index)
        std::cout << "\x1b[" << (y + 1) << ";1H";
        // Batch output per line to minimize terminal writes
        std::string out;
        out.reserve(cols * 20);
        int lastFr = -1, lastFg = -1, lastFb = -1;
        int lastBr = -1, lastBg = -1, lastBb = -1;
        for (int x = 0; x < cols; ++x) {
            const characterSpace& cs = screenBuffer[y][x];
            // Skip continuation/placeholder cells
            if (cs.utf8.empty() && cs.character == '\0') {
                continue;
            }
            // Emit color changes only when needed
            if (lastFr != cs.colorForeground.r || lastFg != cs.colorForeground.g || lastFb != cs.colorForeground.b) {
                lastFr = cs.colorForeground.r; lastFg = cs.colorForeground.g; lastFb = cs.colorForeground.b;
                out += "\x1b[38;2;";
                out += std::to_string((int)cs.colorForeground.r); out += ';';
                out += std::to_string((int)cs.colorForeground.g); out += ';';
                out += std::to_string((int)cs.colorForeground.b); out += 'm';
            }
            if (lastBr != cs.colorBackground.r || lastBg != cs.colorBackground.g || lastBb != cs.colorBackground.b) {
                lastBr = cs.colorBackground.r; lastBg = cs.colorBackground.g; lastBb = cs.colorBackground.b;
                out += "\x1b[48;2;";
                out += std::to_string((int)cs.colorBackground.r); out += ';';
                out += std::to_string((int)cs.colorBackground.g); out += ';';
                out += std::to_string((int)cs.colorBackground.b); out += 'm';
            }
            if (!cs.utf8.empty()) {
                out += cs.utf8; // UTF-8 glyph
            } else {
                out += cs.character ? std::string(1, cs.character) : std::string(" ");
            }
        }
        std::cout << out;
        // No newline; we explicitly position the cursor on each loop
    }
    // Reset attributes and restore wrap mode
    std::cout << "\x1b[0m\x1b[?7h" << std::flush;
}

characterSpace TUImanager::getCharacter(int x, int y){
    return this->screenBuffer[y][x];
}
void TUImanager::drawCharacter(characterSpace character, int x, int y) {
    characterSpace c = getCharacter(x, y);

    float alpha = character.colorBackground.a / 255.0f;
    color bgOld = c.colorBackground;
    color bgNew = character.colorBackground;

    color blendedBg;
    blendedBg.r = static_cast<uint8_t>(bgOld.r * (1.0f - alpha) + bgNew.r * alpha);
    blendedBg.g = static_cast<uint8_t>(bgOld.g * (1.0f - alpha) + bgNew.g * alpha);
    blendedBg.b = static_cast<uint8_t>(bgOld.b * (1.0f - alpha) + bgNew.b * alpha);
    blendedBg.a = 255; // Result is fully opaque

    character.colorBackground = blendedBg;
    // If incoming UTF-8 string is empty and ASCII char is zero, treat as placeholder
    if (character.utf8.empty() && character.character == '\0') {
        // keep placeholder
    }
    this->screenBuffer[y][x] = character;
}

void TUImanager::drawString(const std::string& str, color fg, color bg, int x, int y) {
    if (y < 0 || y >= rows) return;
    // UTF-8 decode and use wcwidth for display width
    mbstate_t ps{};
    const char* s = str.c_str();
    size_t len = str.size();
    int col = x;
    while (len > 0 && col < cols) {
        wchar_t wc;
        size_t consumed = mbrtowc(&wc, s, len, &ps);
        if (consumed == (size_t)-1 || consumed == (size_t)-2) {
            // invalid/partial sequence, render a replacement char
            wc = L'?';
            consumed = 1; // skip one byte to make progress
            // reset state on error
            memset(&ps, 0, sizeof(ps));
        } else if (consumed == 0) {
            break; // null terminator
        }
        int w = wcwidth(wc);
        if (w < 0) w = 1; // Non-printable -> width 1 fallback
        if (col + w > cols) break; // clip at right edge

        // Prepare the primary cell with UTF-8 bytes
        characterSpace cell{};
        cell.character = '\0';
        cell.colorForeground = fg;
        cell.colorBackground = bg;
        cell.utf8.assign(s, consumed);
        drawCharacter(cell, col, y);

        // Fill continuation cells as placeholders (no glyph printed, just colors)
        for (int i = 1; i < w; ++i) {
            if (col + i >= cols) break;
            characterSpace cont{};
            cont.character = '\0';
            cont.colorForeground = fg;
            cont.colorBackground = bg;
            cont.utf8.clear();
            drawCharacter(cont, col + i, y);
        }
        col += std::max(1, w);
        s += consumed;
        len -= consumed;
    }
}

int TUImanager::measureColumns(const std::string& str) {
    mbstate_t ps{};
    const char* s = str.c_str();
    size_t len = str.size();
    int cols = 0;
    while (len > 0) {
        wchar_t wc;
        size_t consumed = mbrtowc(&wc, s, len, &ps);
        if (consumed == (size_t)-1 || consumed == (size_t)-2) {
            // invalid/partial; count as 1 column and skip a byte
            cols += 1;
            consumed = 1;
            memset(&ps, 0, sizeof(ps));
        } else if (consumed == 0) {
            break;
        } else {
            int w = wcwidth(wc);
            if (w < 0) w = 1;
            cols += std::max(1, w);
        }
        s += consumed;
        len -= consumed;
    }
    return cols;
}

void TUImanager::drawBox(int x, int y, int width, int height, color borderFg, color borderBg, color fillBg) {
    if (width < 2 || height < 2) return;
    // Unicode rounded box pieces
    const std::string tl = "╭";
    const std::string tr = "╮";
    const std::string bl = "╰";
    const std::string br = "╯";
    const std::string hor = "─";
    const std::string ver = "│";

    // Top row
    drawString(tl, borderFg, borderBg, x, y);
    for (int i = 1; i < width - 1; ++i) drawString(hor, borderFg, borderBg, x + i, y);
    drawString(tr, borderFg, borderBg, x + width - 1, y);

    // Middle rows
    for (int row = 1; row < height - 1; ++row) {
        drawString(ver, borderFg, borderBg, x, y + row);
        // fill interior with spaces with fillBg
        for (int col = 1; col < width - 1; ++col) {
            drawCharacter({' ', borderFg, fillBg, std::string()}, x + col, y + row);
        }
        drawString(ver, borderFg, borderBg, x + width - 1, y + row);
    }

    // Bottom row
    drawString(bl, borderFg, borderBg, x, y + height - 1);
    for (int i = 1; i < width - 1; ++i) drawString(hor, borderFg, borderBg, x + i, y + height - 1);
    drawString(br, borderFg, borderBg, x + width - 1, y + height - 1);
}

// Switch focused container and update hover visuals
void TUImanager::focusContainer(container* target, int index) {
    if (!target) return;
    // Unhover current focused element and container
    if (containerID) {
        if (auto* curElem = containerID->getFocused()) {
            curElem->notifyHover(*this, false);
        }
        containerID->setHovered(false);
    }

    // Switch
    containerID = target;
    containerID->tui = this;

    // Set focus index and hover element
    if (!containerID->elements.empty()) {
        if (index < 0 || index >= static_cast<int>(containerID->elements.size())) {
            index = 0;
        }
        containerID->focusedIndex = index;
        containerID->getFocused()->notifyHover(*this, true);
    } else {
        containerID->focusedIndex = -1;
    }
    containerID->setHovered(true);
}

// container rendering moved out of header to avoid incomplete type usage
void container::render(TUImanager& tui) {
    color useBg = isHovered ? style.bgHi : style.bg;
    color useFg = isHovered ? style.fgHi : style.fg;

    tui.drawBox(position.x, position.y, size.x, size.y, useFg, useBg, useBg);
    
    for (element* el : elements) {
        el->render(tui);
    }

    if(label != "") {
        tui.drawString("{", useFg, useBg, position.x+1, position.y);
        tui.drawString(label, useFg, useBg, position.x+2, position.y);
        int labelCols = tui.measureColumns(label);
        tui.drawString("}", useFg, useBg, position.x + 2 + labelCols, position.y);
    }
}

// Implementation of container::navigate moved from header to here.
void container::navigate(pressedKey dir) {
    if (elements.empty()) return;
    if (focusedIndex == -1) focusedIndex = 0;  // Start at first

    auto moveWithin = [&](int delta){
        int sz = static_cast<int>(elements.size());
        int next = focusedIndex + delta;
        if (next < 0 || next >= sz) {
            return false; // overflow
        }
        focusedIndex = next;
        updateFocus();
        return true;
    };

    if (dir == UP) {
        // try move up
        if (!moveWithin(-1)) {
            // overflow above
            if (behaviourUp == WRAP) {
                focusedIndex = static_cast<int>(elements.size()) - 1;
                updateFocus();
            } else if (behaviourUp == JUMP && up) {
                // move focus to the up container
                if (tui) elements[focusedIndex]->notifyHover(*tui, false);
                tui->containerID = up;
                up->tui = tui;
                // put focus at last element of up container
                up->focusedIndex = up->elements.empty() ? -1 : static_cast<int>(up->elements.size()) - 1;
                if (up->focusedIndex != -1 && tui) up->getFocused()->notifyHover(*tui, true);
            } // STOP does nothing
        }
    } else if (dir == DOWN) {
        if (!moveWithin(1)) {
            // overflow below
            if (behaviourDown == WRAP) {
                focusedIndex = 0;
                updateFocus();
            } else if (behaviourDown == JUMP && down) {
                if (tui) elements[focusedIndex]->notifyHover(*tui, false);
                tui->containerID = down;
                down->tui = tui;
                down->focusedIndex = down->elements.empty() ? -1 : 0;
                if (down->focusedIndex != -1 && tui) down->getFocused()->notifyHover(*tui, true);
            }
        }
    }
}
