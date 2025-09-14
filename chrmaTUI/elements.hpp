#include "chrmaTUI.hpp"

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