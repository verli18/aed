#include "chrmaTUI.hpp"

class Button : public element {
    public:
        std::string label;
        std::function<void()> onClick;
    
        Button(const std::string& lbl, point pos, int w, int h);
    
        void render(TUImanager& tui) override;
        void onHover(bool isHovered) override;
        void onInteract(pressedKey key) override;
};

class ToggleButton : public element {
    public:
        std::string label;
        bool toggledOn = false;
    
        ToggleButton(const std::string& lbl, point pos, int w, int h);
    
        void render(TUImanager& tui) override;
        void onHover(bool isHovered) override;
        void onInteract(pressedKey key) override;
};
