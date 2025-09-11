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

void TUImanager::clearScreen() {
    // Move cursor to home position only; avoid full clear per frame to reduce flicker
    std::cout << "\x1b[H";
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

bool TUImanager::windowShouldClose() {
    char c;
    int nread = read(STDIN_FILENO, &c, 1);

    if (nread == -1 && errno != EAGAIN) {
        // Handle read error
        return true; 
    }

    if (nread == 1) {
        pressedKey key = UNKNOWN;

        if (c == '\x1b') { // Escape sequence
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) { key = UNKNOWN; }
            else if (read(STDIN_FILENO, &seq[1], 1) != 1) { key = UNKNOWN; }
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
        if (key == UNKNOWN) return false;

        container* current = containerID;
        element* focusedElem = current ? current->getFocused() : nullptr;

        switch (userState) {
            case NAVIGATING:
                if (key == UP || key == DOWN) {
                    if (current) current->navigate(key);
                } else if (key == LEFT && current->left) {
                    containerID = current->left;
                    containerID->focusedIndex = 0;
                    if(containerID->elements.size() > 0) containerID->elements[0]->onHover(true);
                } else if (key == RIGHT && current->right) {
                    containerID = current->right;
                    containerID->focusedIndex = 0;
                    if(containerID->elements.size() > 0) containerID->elements[0]->onHover(true);
                } else if (key == ENTER && focusedElem) {
                    // Change state to interacting and immediately fire the interaction
                    userState = focusedElem->capturesInput() ? CAPTURE : INTERACTING;
                    focusedElem->onInteract(key);
                    // After a toggle, we typically want to remain in NAVIGATING mode
                    userState = NAVIGATING;
                }
                break;
            case CAPTURE: // For text input, etc.
                if (focusedElem) focusedElem->onInteract(key);
                // Add logic here to leave CAPTURE mode, e.g., on Enter
                break;
            case INTERACTING: // This state is now effectively handled in NAVIGATING
                break;
        }
    }
    return false;
}

void TUImanager::render() {
    clearScreen();
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
            out += cs.character;
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
    this->screenBuffer[y][x] = character;
}

void TUImanager::drawString(const std::string& str, color fg, color bg, int x, int y) {
    characterSpace c;
    for (size_t i = 0; i < str.size(); ++i) {
        if (x + i >= cols) break;
        c.character = str[i];
        c.colorForeground = fg;
        c.colorBackground = bg;
        drawCharacter(c, x + i, y);
    }
}
