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

typedef struct standardStyle{
    color fg;
    color bg;
    color fgHi;
    color bgHi;
} standardStyle;

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

enum containerBehaviour{
    WRAP,
    STOP,
    JUMP
};

class element{
    public:

        virtual ~element() = default;
        virtual void render(TUImanager& tui) = 0;  // Pure virtual: must implement
        virtual void onHover(bool isHovered) = 0;
        virtual void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) {}
        virtual void update() {}
        virtual bool capturesInput() { return false; }

    // Unified styling setter for all elements
    inline void setStyle(const standardStyle& s) { style = s; }

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
    standardStyle style; // unified styling for normal and highlight states
        bool isHovered;
};

class container {
    public:
        point position = {0,0};
        point size = {10,5};
        std::string label = "";
        container(point pos, point sz, standardStyle st, std::string lbl)
            : position(pos), size(sz), label(lbl), style(st) {}

        TUImanager* tui = nullptr; // set by owner so we can deliver it to callbacks
    
        std::vector<element*> elements;
        int focusedIndex = 0;
    

        void addElement(element* e) { elements.push_back(e); }
        void removeElement(size_t index) { elements.erase(elements.begin() + index); }

        // Navigate vertically within this container (implemented in .cpp)
        void navigate(pressedKey dir);
    
        // Get the focused element
        element* getFocused() {
            return (focusedIndex >= 0 && focusedIndex < elements.size()) ? elements[focusedIndex] : nullptr;
        }

        void render(TUImanager& tui);
    
        void setContainerBehaviour(containerBehaviour upB, container* up, containerBehaviour downB, container* down) {
            behaviourUp = upB;
            behaviourDown = downB;
            this->up = up;
            this->down = down;
        }

        void setLeft(container* left) { this->left = left; }
        void setRight(container* right) { this->right = right; }

        void setStyle(standardStyle style) { this->style = style; }
    // Visually mark container hovered/focused (affects render border colors)
    inline void setHovered(bool h) { isHovered = h; }
        container* getLeftPointer() { return left; }
        container* getRightPointer() { return right; }
        container* getUpPointer() { return up; }
        container* getDownPointer() { return down; }
    private:

        standardStyle style;
        
        container* left = nullptr;
        container* right = nullptr;
        container* up = nullptr;
        container* down = nullptr;

        containerBehaviour behaviourUp = STOP; //these gotta be private so dumbasses like me don't modify a variable while forgetting the other one
        containerBehaviour behaviourDown = STOP;

        bool isHovered = false;

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
        userState = NAVIGATING;
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
    // Draw a rounded box using Unicode characters. x,y are top-left, width/height in character cells.
    void drawBox(int x, int y, int width, int height, color borderFg, color borderBg, color fillBg);
       
    // Measure how many terminal columns a UTF-8 string will occupy
    int measureColumns(const std::string& str);

    // Focus management: switch active container and element focus safely
    void focusContainer(container* target, int index = 0);
};

#endif // CHRMA_TUI_HPP