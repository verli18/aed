#ifndef CHRMA_TUI_HPP
#define CHRMA_TUI_HPP

class TUImanager; // Forward declaration

#include <termios.h>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <chrono>
#include <cmath>

extern struct termios originalTermios;
struct point{
    int x;
    int y;
};

typedef struct color{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} color;

typedef struct characterSpace{
    char character;
    color colorForeground;
    color colorBackground;
} characterSpace;

enum userState{
    NAVIGATING,
    INTERACTING,
    CAPTURE
};

enum pressedKey{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    Q,
    BACKSPACE,
    ENTER,
    UNKNOWN
};

class element{
    public:

        virtual ~element() = default;
        virtual void render(TUImanager& tui) = 0;  // Pure virtual: must implement
        virtual void onHover(bool isHovered) = 0;
        virtual void onInteract(pressedKey key) {}
        virtual void update() {}
        virtual bool capturesInput() { return false; }
    
    protected:
        point position, size;
        color fg, bg;
        bool isHovered;
};

class container {
    public:
        std::vector<element*> elements;
        container* left = nullptr;
        container* right = nullptr;
        int focusedIndex = -1;
    
        void addElement(element* e) { elements.push_back(e); }
        void removeElement(size_t index) { elements.erase(elements.begin() + index); }
    
        // Navigate vertically within this container
        void navigate(pressedKey dir) {
            if (elements.empty()) return;
            if (focusedIndex == -1) focusedIndex = 0;  // Start at first
            if (dir == UP) focusedIndex = (focusedIndex - 1 + elements.size()) % elements.size();
            else if (dir == DOWN) focusedIndex = (focusedIndex + 1) % elements.size();
            updateFocus();
        }
    
        // Get the focused element
        element* getFocused() {
            return (focusedIndex >= 0 && focusedIndex < elements.size()) ? elements[focusedIndex] : nullptr;
        }
    
    private:
        void updateFocus() {
            for (size_t i = 0; i < elements.size(); ++i) {
                elements[i]->onHover(i == focusedIndex);
            }
        }
    };

void enableRawMode();
void disableRawMode();
void getTerminalSize(int& rows, int& cols);
pressedKey mapCharToKey(char c);

class TUImanager{

    public:
    uint8_t userState;
    element* elementID;
    container* containerID;
    int rows, cols;
    std::vector<std::vector<characterSpace>> screenBuffer;
    TUImanager(){
        enableRawMode();
        std::setbuf(stdout, nullptr);  // Disable stdio buffering
        getTerminalSize(rows, cols);
        screenBuffer.resize(rows, std::vector<characterSpace>(cols));
        std::cout << "[?25l" << std::flush;
    }

    ~TUImanager(){
        disableRawMode();
    }

    void clearScreen();
    void render();
    bool windowShouldClose();
    
    characterSpace getCharacter(int x, int y);
    void drawCharacter(characterSpace, int x, int y);
    void drawString(const std::string& str, color fg, color bg, int x, int y);
};

#endif // CHRMA_TUI_HPP