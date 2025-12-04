#include "chrmaTUI.hpp"

struct termios originalTermios;
// Resolve percent/anchor layout into absolute position and size each frame
void element::applyLayoutForFrame(TUImanager& tui) {
    const LayoutSpec& L = layout;
    int baseX = 0, baseY = 0, availW = tui.cols, availH = tui.rows;
    if (parent) {
        if (L.relativeToInterior) {
            baseX = parent->position.x + 1;
            baseY = parent->position.y + 1;
            availW = std::max(0, parent->size.x - 2);
            availH = std::max(0, parent->size.y - 2);
        } else {
            baseX = parent->position.x;
            baseY = parent->position.y;
            availW = parent->size.x;
            availH = parent->size.y;
        }
    }

    int px, py;
    int pw = size.x;
    int ph = size.y;

    if (L.usePercentX) {
        px = baseX + static_cast<int>(L.percentX * 0.01f * availW);
    } else {
        px = baseX + position.x;
    }
    if (L.usePercentY) {
        py = baseY + static_cast<int>(L.percentY * 0.01f * availH);
    } else {
        py = baseY + position.y;
    }

    px += L.offsetX;
    py += L.offsetY;

    if (L.usePercentW) pw = std::max(L.minW, static_cast<int>(L.percentW * 0.01f * availW));
    if (L.usePercentH) ph = std::max(L.minH, static_cast<int>(L.percentH * 0.01f * availH));

    // Apply anchor adjustments
    if (L.anchorX == AnchorX::Center) px -= pw / 2;
    else if (L.anchorX == AnchorX::Right) px -= pw;

    if (L.anchorY == AnchorY::Middle) py -= ph / 2;
    else if (L.anchorY == AnchorY::Bottom) py -= ph;

    renderPos.x = px;
    renderPos.y = py;
    if (L.usePercentW) size.x = pw;
    if (L.usePercentH) size.y = ph;
}

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
    // Reset internal buffer to printable spaces with provided background
    color fg = {0, 0, 0, 255};
    // All cells become dirty after clear
    dirtyCount = static_cast<size_t>(rows) * static_cast<size_t>(cols);
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            // Use a printable space so render() emits a colored cell instead of skipping it
            characterSpace cell{};
            cell.character = ' ';
            cell.colorForeground = fg;
            cell.colorBackground = col;
            cell.z = 0;
            cell.utf8.clear();
            screenBuffer[y][x] = cell;
            dirty[y][x] = 1;
        }
    }
}

// Clear a rectangular region (width x height) starting at x,y with background color bg.
// This writes space glyphs into the internal buffer and marks cells dirty so render() will update the terminal.
void TUImanager::clearRect(int x, int y, int width, int height, color bg) {
    if (width <= 0 || height <= 0) return;
    int startX = std::max(0, x);
    int startY = std::max(0, y);
    int endX = std::min(cols, x + width);
    int endY = std::min(rows, y + height);
    color fg = {0,0,0,255};
    for (int yy = startY; yy < endY; ++yy) {
        for (int xx = startX; xx < endX; ++xx) {
            characterSpace cell{};
            cell.character = ' ';
            cell.colorForeground = fg;
            cell.colorBackground = bg;
            cell.z = 0;
            cell.utf8.clear();
            screenBuffer[yy][xx] = cell;
            if (!dirty[yy][xx]) { dirty[yy][xx] = 1; ++dirtyCount; }
        }
    }
}

pressedKey mapCharToKey(char c){
    switch (c) {
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
            // Read the rest of the sequence non-blocking up to a short limit
            char buf[32];
            int bi = 0;
            // Try to read up to buf size or until a likely terminator is seen
            for (int i = 0; i < 31; ++i) {
                int r = read(STDIN_FILENO, &buf[bi], 1);
                if (r != 1) break;
                char ch = buf[bi];
                bi++;
                // Common terminators for CSI sequences: letters and tilde
                if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '~') break;
            }
            
            if (bi == 0) {
                // No additional bytes read - this is a standalone ESC key
                key = ESC;
            } else {
                std::string seq(buf, buf + bi);
                if (!seq.empty() && seq[0] == '[') {
                    // Simple arrow keys (CSI A/B/C/D)
                    if (bi >= 2 && (seq[1] == 'A' || seq[1] == 'B' || seq[1] == 'C' || seq[1] == 'D')) {
                        switch (seq[1]) {
                            case 'A': key = UP; break;
                            case 'B': key = DOWN; break;
                            case 'C': key = RIGHT; break;
                            case 'D': key = LEFT; break;
                        }
                    } else {
                        // Look for various Shift+Enter patterns
                        // Common patterns: "13;2u", "13;2~", "1;2M" (some terminals), "13:2u" (kitty)
                        if (seq.find("13;2u") != std::string::npos || 
                            seq.find("13;2~") != std::string::npos ||
                            seq.find("13:2u") != std::string::npos ||
                            seq.find("1;2M") != std::string::npos) {
                            key = SHIFT_ENTER;
                        }
                    }
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

bool TUImanager::waitForInput(int timeoutMs) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    int ret = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &tv);
    if (ret > 0 && FD_ISSET(STDIN_FILENO, &readfds)) return true;
    return false;
}

bool TUImanager::windowShouldClose() {
    return pollInput();
}

void TUImanager::render() {
    // Disable line wrap to avoid auto-wrapping the last column into the next line, too stupid to fix this the right way
    std::cout << "\x1b[?7l";
    for (int y = 0; y < rows; ++y) {
        int lastFr = -1, lastFg = -1, lastFb = -1;
        int lastBr = -1, lastBg = -1, lastBb = -1;
        int x = 0;
        while (x < cols) {
            // Skip clean cells and placeholders
            while (x < cols) {
                if (!dirty[y][x]) { ++x; continue; }
                const characterSpace& cs = screenBuffer[y][x];
                if (cs.utf8.empty() && cs.character == '\0') { dirty[y][x] = 0; ++x; continue; }
                break;
            }
            if (x >= cols) break;

            // Start of a dirty printable span
            int startX = x;
            std::cout << "\x1b[" << (y + 1) << ";" << (startX + 1) << "H";

            std::string out;
            out.reserve((cols - startX) * 8);
            while (x < cols && dirty[y][x]) {
                const characterSpace& cs = screenBuffer[y][x];
                // Placeholders are cleared and break the span visually (no output)
                if (cs.utf8.empty() && cs.character == '\0') { if (dirty[y][x]) { dirty[y][x] = 0; if (dirtyCount) --dirtyCount; } break; }
                // Emit color changes inline in the span buffer
                if (lastFr != cs.colorForeground.r || lastFg != cs.colorForeground.g || lastFb != cs.colorForeground.b) {
                    lastFr = cs.colorForeground.r; lastFg = cs.colorForeground.g; lastFb = cs.colorForeground.b;
                    out += "\x1b[38;2"; out += ";"; out += std::to_string((int)cs.colorForeground.r);
                    out += ";"; out += std::to_string((int)cs.colorForeground.g);
                    out += ";"; out += std::to_string((int)cs.colorForeground.b); out += "m";
                }
                if (lastBr != cs.colorBackground.r || lastBg != cs.colorBackground.g || lastBb != cs.colorBackground.b) {
                    lastBr = cs.colorBackground.r; lastBg = cs.colorBackground.g; lastBb = cs.colorBackground.b;
                    out += "\x1b[48;2"; out += ";"; out += std::to_string((int)cs.colorBackground.r);
                    out += ";"; out += std::to_string((int)cs.colorBackground.g);
                    out += ";"; out += std::to_string((int)cs.colorBackground.b); out += "m";
                }
                if (!cs.utf8.empty()) out += cs.utf8; else out += cs.character ? std::string(1, cs.character) : std::string(" ");
                if (dirty[y][x]) { dirty[y][x] = 0; if (dirtyCount) --dirtyCount; }
                ++x;
            }
            std::cout << out;
        }
    }
    // Reset attributes and restore wrap mode
    std::cout << "\x1b[0m\x1b[?7h" << std::flush;
}

characterSpace TUImanager::getCharacter(int x, int y){
    return this->screenBuffer[y][x];
}
void TUImanager::drawCharacter(characterSpace character, int x, int y) {
    if (x < 0 || x >= cols || y < 0 || y >= rows) return;
    const characterSpace& existing = screenBuffer[y][x];
    int incomingZ = currentZ;
    // Respect z-order: only draw if we are at or above the existing z
    if (incomingZ < existing.z) {
        return;
    }

    // Compose final background with fast paths
    color finalBg;
    uint8_t a = character.colorBackground.a;
    if (a == 0) {
        finalBg = existing.colorBackground; // fully transparent => keep old bg
    } else if (a == 255) {
        finalBg = character.colorBackground; // fully opaque => replace
    } else {
        float alpha = a / 255.0f;
        const color& bgOld = existing.colorBackground;
        const color& bgNew = character.colorBackground;
        finalBg.r = static_cast<uint8_t>(bgOld.r * (1.0f - alpha) + bgNew.r * alpha);
        finalBg.g = static_cast<uint8_t>(bgOld.g * (1.0f - alpha) + bgNew.g * alpha);
        finalBg.b = static_cast<uint8_t>(bgOld.b * (1.0f - alpha) + bgNew.b * alpha);
        finalBg.a = 255;
    }

    character.colorBackground = finalBg;
    character.z = incomingZ;

    // Skip write if nothing changes
    bool sameUtf8 = (character.utf8 == existing.utf8);
    bool sameChar = (character.character == existing.character);
    bool sameFg = std::memcmp(&character.colorForeground, &existing.colorForeground, sizeof(color)) == 0;
    bool sameBg = std::memcmp(&character.colorBackground, &existing.colorBackground, sizeof(color)) == 0;
    bool sameZ = (character.z == existing.z);
    if (sameUtf8 && sameChar && sameFg && sameBg && sameZ) {
        return;
    }

    screenBuffer[y][x] = character;
    if (!dirty[y][x]) { dirty[y][x] = 1; ++dirtyCount; }
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
            characterSpace fillCell{};
            fillCell.character = ' ';
            fillCell.colorForeground = borderFg;
            fillCell.colorBackground = fillBg;
            drawCharacter(fillCell, x + col, y + row);
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
        // Clear the previous container's area in the buffer so anything drawn above it (e.g., a popup)
        // doesn't leave remnants when that top container is hidden. Use the previous container's background.
        // We clear using its style.bg so the terminal shows the container's background when re-rendered.
        containerID->setHovered(false);
        // compute area to clear: full container box
    standardStyle prevStyle = containerID->getStyle();
    clearRect(containerID->position.x, containerID->position.y, containerID->size.x, containerID->size.y, prevStyle.bg);
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
    // If this container is the active one in the TUImanager, treat it as hovered so
    // it uses the highlight colors even if setHovered() was not invoked.
    bool active = isHovered || (tui.containerID == this);
    color useBg = active ? style.bgHi : style.bg;
    color useFg = active ? style.fgHi : style.fg;

    // Set z for this container rendering
    int prevZ = tui.getCurrentZ();
    tui.setCurrentZ(zIndex);

    if (renderBox) {
        tui.drawBox(position.x, position.y, size.x, size.y, useFg, useBg, useBg);
    } else {
        // If not rendering the box, still clear the interior area so children render on a clean background.
        tui.clearRect(position.x + 1, position.y + 1, std::max(0, size.x - 2), std::max(0, size.y - 2), useBg);
    }

    for (element* el : elements) {
        el->applyLayoutForFrame(tui);
        el->render(tui);
    }

    if (renderBox && label != "") {
        tui.drawString("{", useFg, useBg, position.x+1, position.y);
        tui.drawString(label, useFg, useBg, position.x+2, position.y);
        int labelCols = tui.measureColumns(label);
        tui.drawString("}", useFg, useBg, position.x + 2 + labelCols, position.y);
    }

    // Restore previous z
    tui.setCurrentZ(prevZ);
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
        // Skip non-focusable elements
        while (next >= 0 && next < sz && !elements[next]->canBeFocused()) {
            next += delta;
        }
        if (next < 0 || next >= sz) return false;
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
                while (focusedIndex >= 0 && !elements[focusedIndex]->canBeFocused()) {
                    focusedIndex--;
                }
                if (focusedIndex < 0) focusedIndex = 0; // fallback to first, even if not focusable
                updateFocus();
            } else if (behaviourUp == JUMP && up) {
                // move focus to the up container
                if (tui) elements[focusedIndex]->notifyHover(*tui, false);
                tui->containerID = up;
                up->tui = tui;
                // put focus at last focusable element of up container
                up->focusedIndex = static_cast<int>(up->elements.size()) - 1;
                while (up->focusedIndex >= 0 && !up->elements[up->focusedIndex]->canBeFocused()) {
                    up->focusedIndex--;
                }
                if (up->focusedIndex < 0) up->focusedIndex = -1;
                if (up->focusedIndex != -1 && tui) up->getFocused()->notifyHover(*tui, true);
            } // STOP does nothing
        }
    } else if (dir == DOWN) {
        if (!moveWithin(1)) {
            // overflow below
            if (behaviourDown == WRAP) {
                focusedIndex = 0;
                while (focusedIndex < static_cast<int>(elements.size()) && !elements[focusedIndex]->canBeFocused()) {
                    focusedIndex++;
                }
                if (focusedIndex >= static_cast<int>(elements.size())) focusedIndex = static_cast<int>(elements.size()) - 1; // fallback
                updateFocus();
            } else if (behaviourDown == JUMP && down) {
                if (tui) elements[focusedIndex]->notifyHover(*tui, false);
                tui->containerID = down;
                down->tui = tui;
                down->focusedIndex = 0;
                while (down->focusedIndex < static_cast<int>(down->elements.size()) && !down->elements[down->focusedIndex]->canBeFocused()) {
                    down->focusedIndex++;
                }
                if (down->focusedIndex >= static_cast<int>(down->elements.size())) down->focusedIndex = -1;
                if (down->focusedIndex != -1 && tui) down->getFocused()->notifyHover(*tui, true);
            }
        }
    }
}

void TUImanager::markDirty(int x, int y, int width, int height) {
    for (int dy = 0; dy < height; ++dy) {
        int yy = y + dy;
        if (yy < 0 || yy >= rows) continue;
        for (int dx = 0; dx < width; ++dx) {
            int xx = x + dx;
            if (xx < 0 || xx >= cols) continue;
            if (!dirty[yy][xx]) {
                dirty[yy][xx] = 1;
                ++dirtyCount;
            }
            // Reset the cell to background so it gets redrawn
            screenBuffer[yy][xx].z = -1;
        }
    }
}

void TUImanager::markAllDirty() {
    dirtyCount = static_cast<size_t>(rows) * static_cast<size_t>(cols);
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            dirty[y][x] = 1;
            screenBuffer[y][x].z = -1;
        }
    }
}