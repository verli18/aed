#include "chrmaTUI.hpp"
#include <unistd.h>
#include <chrono>

class Button : public element {
    public:
        std::string label;
        std::function<void()> onClick;
    
        Button(const std::string& lbl, point pos, int w, int h);
    
        void render(TUImanager& tui) override;
        void onHover(bool isHovered) override;
        void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) override;
};

class ToggleButton : public element {
    public:
        std::string label;
        bool toggledOn = false;
    
        ToggleButton(const std::string& lbl, point pos, int w, int h);
        // Use base element::setStyle for styling
    
        void render(TUImanager& tui) override;
        void onHover(bool isHovered) override;
    void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) override;
};

class InputBar : public element {
public:
    std::string text;
    std::string label;

    InputBar(const std::string& lbl, point pos, int w, int h);
        // Use base element::setStyle for styling

    void render(TUImanager& tui) override;
    void onHover(bool isHovered) override;
    void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) override;
    bool capturesInput() override { return true; }
};

class Slider : public element {
public:
    float value;
    float step;
    float minValue;
    float maxValue;

    Slider(float min, float max, float initialValue, float step, point pos, int w, int h);
        // Use base element::setStyle for styling

    void render(TUImanager& tui) override;
    void onHover(bool isHovered) override;
    void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) override;
    bool capturesInput() override { return true; }
};

class Selector : public element {
public:
    std::vector<std::string> options;
    int selectedIndex;
    std::string label;

    Selector(const std::string& lbl, const std::vector<std::string>& opts, point pos, int w, int h);
        // Use base element::setStyle for styling

    void render(TUImanager& tui) override;
    void onHover(bool isHovered) override;
    void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) override;
    bool capturesInput() override { return true; }
    
    std::string getSelectedOption() const;
};

class DropdownMenu : public element {
public:
    std::vector<std::string> options;
    int selectedIndex;
    std::string label;
    bool isOpen;
    int maxOpenHeight;

    DropdownMenu(const std::string& lbl, const std::vector<std::string>& opts, point pos, int w, int h, int maxHeight = 8);
        // Use base element::setStyle for styling

    void render(TUImanager& tui) override;
    void onHover(bool isHovered) override;
    void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) override;
    bool capturesInput() override { return true; }
    
    std::string getSelectedOption() const;
};

class Text : public element {
public:
    std::string content;
    bool multiLine = false;  // Enable multi-line rendering
    int maxWidth = 0;        // Maximum width for word wrapping (0 = no limit)

    // Absolute ctor
    Text(const std::string& text, point pos) : content(text) {
        position = pos;
        size = {0, 1};
        isHovered = false;
    }
    
    // Absolute ctor with multi-line support
    Text(const std::string& text, point pos, bool multiLine, int maxWidth = 0) : content(text), multiLine(multiLine), maxWidth(maxWidth) {
        position = pos;
        size = {0, 1};
        isHovered = false;
    }
    
    // Percent ctor (relative to parent container's interior area)
    Text(const std::string& text, float percentX, float percentY) : content(text) {
        position = {0,0};
        size = {0, 1};
        isHovered = false;
        setPercentPosition(percentX, percentY);
    }
    
    // Percent ctor with multi-line support
    Text(const std::string& text, float percentX, float percentY, bool multiLine, int maxWidth = 0) : 
        content(text), multiLine(multiLine), maxWidth(maxWidth) {
        position = {0,0};
        size = {0, 1};
        isHovered = false;
        setPercentPosition(percentX, percentY);
    }

    void render(TUImanager& tui) override;
    void onHover(bool hovered) override { isHovered = hovered; }
    bool canBeFocused() const override { return false; }
    
private:
    std::vector<std::string> wrapText(const std::string& text, int maxWidth, TUImanager& tui) const;
    int measureTextWidth(const std::string& text, TUImanager& tui) const;
};

// Multi-line text input with wrapping, arrow navigation, Shift+Enter for newline,
// and Enter to exit capture mode.
class MultiLineInput : public element {
public:
    std::string label;
    std::string text;
    int caretIndex = 0;   // insertion point in text
    int scrollY = 0;      // first visible wrapped line
    
    // Cursor blinking
    mutable std::chrono::steady_clock::time_point lastBlinkTime;
    mutable bool cursorVisible = true;

    MultiLineInput(const std::string& lbl, point pos, int w, int h) : label(lbl) {
        position = pos;
        size = {w, h};
        style.fg = {255, 255, 255, 255};
        style.bg = {50, 50, 50, 255};
        style.fgHi = {255, 255, 255, 255};
        style.bgHi = {70, 70, 70, 255};
        isHovered = false;
        lastBlinkTime = std::chrono::steady_clock::now();
    }

    void render(TUImanager& tui) override;
    void onHover(bool hovered) override { isHovered = hovered; }
    void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) override;
    bool capturesInput() override { return true; }

private:
    struct Wrapped {
        std::vector<std::string> lines;   // visible lines without newlines
        std::vector<int> starts;          // starting text index for each wrapped line
    };

    Wrapped wrap(const std::string& t, int maxWidth, TUImanager& tui) const;
    std::pair<int,int> caretWrappedPos(const Wrapped& w) const; // (lineIdx, col)
};

// Simple RadioButton element. Radio buttons are grouped by a string group name;
// selecting one radio button will deselect others in the same group within the same container.
class RadioButton : public element {
public:
    std::string label;
    std::string group; // grouping key
    bool selected = false;

    RadioButton(const std::string& lbl, const std::string& grp, point pos, int w, int h);

    void render(TUImanager& tui) override;
    void onHover(bool hovered) override;
    void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) override;
};

// ListView: scrollable single-select list with keyboard navigation.
// Similar to DropdownMenu but always expanded and more suitable for data display.
class ListView : public element {
public:
    std::vector<std::string> items;
    int selectedIndex;
    std::string label;
    int visibleRows; // Number of rows to display
    int scrollOffset; // Current scroll position

    ListView(const std::string& lbl, const std::vector<std::string>& itemList, point pos, int w, int h, int visibleRows = 10);

    void render(TUImanager& tui) override;
    void onHover(bool hovered) override;
    void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) override;
    bool capturesInput() override { return true; }
    
    std::string getSelectedItem() const;
    int getSelectedIndex() const { return selectedIndex; }
    
    // Update items dynamically (useful for search/filter)
    void setItems(const std::vector<std::string>& newItems);
};

// RichListItem: multi-line data with per-item theming
struct RichListItem {
    std::vector<std::string> lines;  // Multi-line content for this item
    standardStyle theme;              // Custom colors for this item
    int64_t id = 0;                   // Optional database ID for this item
    
    RichListItem(const std::vector<std::string>& lines, const standardStyle& theme, int64_t id = 0)
        : lines(lines), theme(theme), id(id) {}
};

// RichListView: scrollable list with multi-line items rendered as mini-containers.
// Each item is drawn with a rounded box and can have custom theming.
class RichListView : public element {
public:
    std::vector<RichListItem> items;
    int selectedIndex;
    std::string label;
    int scrollOffset;
    int itemHeight; // Height per item (including border)

    RichListView(const std::string& lbl, const std::vector<RichListItem>& itemList, 
                 point pos, int w, int h, int itemHeight = 5);

    void render(TUImanager& tui) override;
    void onHover(bool hovered) override;
    void onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) override;
    bool capturesInput() override { return true; }
    
    int getSelectedIndex() const { return selectedIndex; }
    const RichListItem* getSelectedItem() const;
    
    // Update items dynamically
    void setItems(const std::vector<RichListItem>& newItems);
};

// ==================== NOTIFICATION SYSTEM ====================

enum class NotificationType {
    Info,
    Success,
    Warning,
    Error
};

struct Notification {
    std::string message;
    NotificationType type;
    std::chrono::steady_clock::time_point created;
    int durationMs;
    
    Notification(const std::string& msg, NotificationType t, int duration = 3000)
        : message(msg), type(t), created(std::chrono::steady_clock::now()), durationMs(duration) {}
    
    bool isExpired() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - created).count();
        return elapsed >= durationMs;
    }
};

// NotificationManager: renders toast-style notifications in the top-right corner
class NotificationManager {
public:
    void push(const std::string& message, NotificationType type = NotificationType::Info, int durationMs = 3000);
    void pushInfo(const std::string& message) { push(message, NotificationType::Info); }
    void pushSuccess(const std::string& message) { push(message, NotificationType::Success); }
    void pushWarning(const std::string& message) { push(message, NotificationType::Warning); }
    void pushError(const std::string& message) { push(message, NotificationType::Error); }
    
    void render(TUImanager& tui);
    bool update(TUImanager& tui); // Remove expired notifications, marks area dirty if any were removed
    bool empty() const { return notifications.empty(); }
    
    // Clear the notification display area (marks it dirty for redraw)
    void clearArea(TUImanager& tui);
    
    int maxNotifications = 5;  // Max visible at once
    int notificationWidth = 35; // Width of notification boxes
    
private:
    std::vector<Notification> notifications;
    int lastRenderedHeight = 0; // Track how much vertical space was used
    
    color getBackgroundColor(NotificationType type) const;
    color getForegroundColor(NotificationType type) const;
};