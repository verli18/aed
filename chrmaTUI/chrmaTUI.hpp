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
#include <locale.h>

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
    // If non-empty, this contains the UTF-8 bytes to render for this cell.
    // If empty and character=='\0', the cell is a continuation/placeholder and nothing is printed.
    // If empty and character!='\0', the ASCII character is rendered.
    std::string utf8;
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
        virtual void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) {}
        virtual void update() {}
        virtual bool capturesInput() { return false; }

        // User-provided callbacks
        // Handlers receive the source element and the TUImanager for rendering
        std::function<void(element&, TUImanager&, bool)> onHoverHandler = nullptr;   // arg: isHovered
        std::function<void(element&, TUImanager&)> onClickHandler = nullptr;
        std::function<void(element&, TUImanager&)> onCaptureEndHandler = nullptr;

        void setHoverHandler(std::function<void(element&, TUImanager&, bool)> handler) {
            onHoverHandler = handler;
        }
        void setClickHandler(std::function<void(element&, TUImanager&)> handler) {
            onClickHandler = handler;
        }
        void setCaptureEndHandler(std::function<void(element&, TUImanager&)> handler) {
            onCaptureEndHandler = handler;
        }
        // Notifiers to invoke virtual handlers and user callbacks
        void notifyHover(TUImanager& tui, bool hovered) {
            isHovered = hovered;
            onHover(hovered);
            if (onHoverHandler) onHoverHandler(*this, tui, hovered);
        }
        void notifyClick(TUImanager& tui) {
            if (onClickHandler) onClickHandler(*this, tui);
        }
        void notifyCaptureEnd(TUImanager& tui) {
            if (onCaptureEndHandler) onCaptureEndHandler(*this, tui);
        }
    
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
    TUImanager* tui = nullptr; // set by owner so we can deliver it to callbacks
    
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

        void render(TUImanager& tui) {
            for (element* el : elements) {
                el->render(tui);
            }
        }
    
    private:
        void updateFocus() {
            for (size_t i = 0; i < elements.size(); ++i) {
                if (tui) {
                    elements[i]->notifyHover(*tui, i == focusedIndex);
                } else {
                    elements[i]->onHover(i == focusedIndex);
                }
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
    // End-of-frame callbacks to run after elements render and before final render()
    std::vector<std::function<void(TUImanager&)>> endOfFrameCallbacks;
    TUImanager(){
        enableRawMode();
        std::setbuf(stdout, nullptr);  // Disable stdio buffering
    // Enable UTF-8 locale so mbrtowc/wcwidth work as expected
    setlocale(LC_ALL, "");
        getTerminalSize(rows, cols);
        screenBuffer.resize(rows, std::vector<characterSpace>(cols));
        std::cout << "[?25l" << std::flush;
    }

    ~TUImanager(){
        disableRawMode();
        std::cout << "\x1b[?25h\x1b[0m\x1b[2J\x1b[H" << std::flush;
    }

    void clearScreen(color col);
    void render();
    // Polls input and updates internal state. Returns true if the app should close.
    bool pollInput();
    // Backwards-compatible name (deprecated): calls pollInput().
    bool windowShouldClose();
    // Schedule a function to run at the end of the current frame, before render().
    inline void enqueueEndOfFrame(std::function<void(TUImanager&)> fn) { endOfFrameCallbacks.push_back(std::move(fn)); }
    // Run and clear all end-of-frame callbacks.
    inline void runEndOfFrame() {
        for (auto &fn : endOfFrameCallbacks) { fn(*this); }
        endOfFrameCallbacks.clear();
    }
    
    characterSpace getCharacter(int x, int y);
    void drawCharacter(characterSpace, int x, int y);
    void drawString(const std::string& str, color fg, color bg, int x, int y);
};

#endif // CHRMA_TUI_HPP