#ifndef CHRMA_TUI_HPP
#define CHRMA_TUI_HPP

class TUImanager; // Forward declaration
class container;   // Forward declaration for parent linkage in element

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
    int z = 0; // z-order for layering (higher draws over lower)
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
    SHIFT_ENTER,
    Q,
    BACKSPACE,
    ENTER,
    ESC,
    UNKNOWN
};

enum containerBehaviour{
    WRAP,
    STOP,
    JUMP
};

class element{
    public:
        // Generic layout for all elements: percent-based or absolute offsets with anchors
        enum class AnchorX { Left, Center, Right };
        enum class AnchorY { Top, Middle, Bottom };
        struct LayoutSpec {
            bool usePercentX = false;
            bool usePercentY = false;
            float percentX = 0.0f; // 0..100
            float percentY = 0.0f; // 0..100
            int offsetX = 0;
            int offsetY = 0;
            bool usePercentW = false;
            bool usePercentH = false;
            float percentW = 0.0f; // 0..100
            float percentH = 0.0f; // 0..100
            int minW = 0;
            int minH = 0;
            AnchorX anchorX = AnchorX::Left;
            AnchorY anchorY = AnchorY::Top;
            bool relativeToInterior = true; // true: inside container box; false: container outer
        };

        virtual ~element() = default;
        virtual void render(TUImanager& tui) = 0;  // Pure virtual: must implement
        virtual void onHover(bool isHovered) = 0;
    virtual void onInteract(pressedKey /*key*/, char /*c*/, uint8_t& /*userState*/, TUImanager& /*tui*/) {}
        virtual void update() {}
        virtual bool capturesInput() { return false; }
        virtual bool canBeFocused() const { return true; }

    // Compute and apply percent/anchor-based layout for this frame
    void applyLayoutForFrame(TUImanager& tui);

    // Unified styling setter for all elements
    inline void setStyle(const standardStyle& s) { style = s; hasCustomStyle = true; }
    inline bool hasStyle() const { return hasCustomStyle; }
    inline void setParent(container* p) { parent = p; }
    inline container* getParent() const { return parent; }

        // Layout setters
        inline void setPercentPosition(float px, float py) { layout.usePercentX = true; layout.usePercentY = true; layout.percentX = px; layout.percentY = py; }
        inline void setPercentX(float px) { layout.usePercentX = true; layout.percentX = px; }
        inline void setPercentY(float py) { layout.usePercentY = true; layout.percentY = py; }
        inline void setOffsets(int ox, int oy) { layout.offsetX = ox; layout.offsetY = oy; }
        inline void setPercentSize(float pw, float ph) { layout.usePercentW = true; layout.usePercentH = true; layout.percentW = pw; layout.percentH = ph; }
        inline void setPercentW(float pw) { layout.usePercentW = true; layout.percentW = pw; }
        inline void setPercentH(float ph) { layout.usePercentH = true; layout.percentH = ph; }
        inline void setMinSize(int w, int h) { layout.minW = w; layout.minH = h; }
        inline void setAnchors(AnchorX ax, AnchorY ay) { layout.anchorX = ax; layout.anchorY = ay; }
        inline void setRelativeToInterior(bool v) { layout.relativeToInterior = v; }
        inline const LayoutSpec& getLayout() const { return layout; }

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

        point renderPos; // Calculated absolute position for rendering.
    
    protected:
        point position, size;
    standardStyle style; // unified styling for normal and highlight states
    container* parent = nullptr; // owning container for relative layout
    bool hasCustomStyle = false; // whether user explicitly set a style
    LayoutSpec layout; // generic layout for all elements
        bool isHovered;
};

class container {
    public:
        point position = {0,0};
        point size = {10,5};
        std::string label = "";
        container(point pos, point sz, standardStyle st, std::string lbl)
            : position(pos), size(sz), label(lbl), style(st), defaultElementStyle(st) {}

    // Stacking order for this container (higher draws above lower)
    inline void setZIndex(int z) { zIndex = z; }
    inline int getZIndex() const { return zIndex; }

    TUImanager* tui = nullptr; // set by owner so we can deliver it to callbacks
    
        std::vector<element*> elements;
        int focusedIndex = 0;
        // Controls whether the container draws its rounded box and label.
        // Set to false for invisible containers used only for navigation/layout.
        bool renderBox = true;
    
        void addElement(element* e) {
            if (!e) return;
            e->setParent(this);
            if (inheritStyle && !e->hasStyle()) {
                e->setStyle(defaultElementStyle);
            }
            elements.push_back(e);
        }
        void removeElement(size_t index) { elements.erase(elements.begin() + index); }

        // Navigate vertically within this container (implemented in .cpp)
        void navigate(pressedKey dir);
    
        // Get the focused element
        element* getFocused() {
            return (focusedIndex >= 0 && focusedIndex < static_cast<int>(elements.size())) ? elements[focusedIndex] : nullptr;
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

    // Set container visibility: when hidden, the container will not draw its box or label
    inline void isHiddenContainer(bool hidden) { renderBox = !hidden; }

    void setStyle(standardStyle style) { this->style = style; }
    void setDefaultElementStyle(const standardStyle& s) { defaultElementStyle = s; }
    void setInheritStyle(bool enabled) { inheritStyle = enabled; }
    // Expose style for read-only access (used by TUImanager for clearing/redrawing)
    inline standardStyle getStyle() const { return style; }
    // Visually mark container hovered/focused (affects render border colors)
    inline void setHovered(bool h) { isHovered = h; }
        container* getLeftPointer() { return left; }
        container* getRightPointer() { return right; }
        container* getUpPointer() { return up; }
        container* getDownPointer() { return down; }
    private:

        standardStyle style;
    int zIndex = 0;
    standardStyle defaultElementStyle; // default for children if they don't override
    bool inheritStyle = true; // apply defaultElementStyle on add
        
        container* left = nullptr;
        container* right = nullptr;
        container* up = nullptr;
        container* down = nullptr;

        containerBehaviour behaviourUp = STOP; //these gotta be private so dumbasses like me don't modify a variable while forgetting the other one
        containerBehaviour behaviourDown = STOP;

        bool isHovered = false;

        void updateFocus() {
                for (size_t i = 0; i < elements.size(); ++i) {
                bool shouldHover = (static_cast<int>(i) == focusedIndex) && elements[i]->canBeFocused();
                if (tui) {
                    elements[i]->notifyHover(*tui, shouldHover);
                } else {
                    elements[i]->onHover(shouldHover);
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
    std::vector<std::vector<uint8_t>> dirty; // dirty flags per cell
    int currentZ = 0; // z for subsequent draw operations
    size_t dirtyCount = 0; // total number of dirty cells
    // End-of-frame callbacks to run after elements render and before final render()
    std::vector<std::function<void(TUImanager&)>> endOfFrameCallbacks;

    TUImanager(){
        enableRawMode();
        std::setbuf(stdout, nullptr);  // Disable stdio buffering
        // Enable UTF-8 locale so mbrtowc/wcwidth work as expected
        setlocale(LC_ALL, "");
        getTerminalSize(rows, cols);
    screenBuffer.resize(rows, std::vector<characterSpace>(cols));
    dirty.assign(rows, std::vector<uint8_t>(cols, 1));
        dirtyCount = static_cast<size_t>(rows) * static_cast<size_t>(cols);
        std::cout << "[?25l" << std::flush;
        userState = NAVIGATING;
    }

    ~TUImanager(){
        disableRawMode();
        std::cout << "\x1b[?25h\x1b[0m\x1b[2J\x1b[H" << std::flush;
    }

    void clearScreen(color col);
    // Clear a rectangular region of the internal buffer and mark cells dirty.
    void clearRect(int x, int y, int width, int height, color bg);
    void render();
    // Polls input and updates internal state. Returns true if the app should close.
    bool pollInput();
    // Block until input or timeout (ms). Returns true if input is ready, false on timeout.
    bool waitForInput(int timeoutMs);
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

    // Z control helpers
    inline void setCurrentZ(int z) { currentZ = z; }
    inline int getCurrentZ() const { return currentZ; }
    inline bool hasDirty() const { return dirtyCount > 0; }
    
    // Mark a rectangular region as dirty (needs redraw)
    void markDirty(int x, int y, int width, int height);
    
    // Mark entire screen as dirty
    void markAllDirty();
};

#endif // CHRMA_TUI_HPP