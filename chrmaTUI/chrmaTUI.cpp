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
    // Move cursor to home position only; avoid full clear per frame to reduce flicker
    std::cout << "\x1b[H";
    for(int y = 0; y < rows; ++y) {
        for(int x = 0; x < cols; ++x) {
            screenBuffer[y][x] = {'\0', {0,0,0,0}, col, std::string()};
        }
    }
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

    if (key == Q) return true;

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
            } else if (key == LEFT && current && current->left) {
                current->getFocused()->notifyHover(*this, false); // Unhover old
                containerID = current->left;
                containerID->tui = this;
                containerID->focusedIndex = 0;
                if(!containerID->elements.empty()) containerID->getFocused()->notifyHover(*this, true); // hover new
            } else if (key == RIGHT && current && current->right) {
                current->getFocused()->notifyHover(*this, false); // Unhover old
                containerID = current->right;
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
