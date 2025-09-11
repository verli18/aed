#include "elements.hpp"

Button::Button(const std::string& lbl, point pos, int w, int h) : label(lbl) {
    position = pos;
    size = {w, h};
    fg = {255, 255, 255, 255};  // White
    bg = {0, 0, 255, 255};     // Blue
    isHovered = false;
}

void Button::render(TUImanager& tui) {
    color renderBg = isHovered ? bg : color{static_cast<uint8_t>(bg.r / 2), static_cast<uint8_t>(bg.g / 2), static_cast<uint8_t>(bg.b / 2), bg.a};  // Dim when not focused
    tui.drawString(label, fg, renderBg, position.x, position.y);
}

void Button::onHover(bool hovered) {
    isHovered = hovered;
}

void Button::onInteract(pressedKey key) {
    if (key == ENTER && onClick) onClick();
}

ToggleButton::ToggleButton(const std::string& lbl, point pos, int w, int h) : label(lbl) {
    position = pos;
    size = {w, h};
    fg = {255, 255, 255, 255};  // White
    bg = {80, 80, 80, 255};     // Dark Gray
    isHovered = false;
}

void ToggleButton::render(TUImanager& tui) {
    color renderBg = isHovered ? color{120, 120, 120, 255} : bg;
    std::string text = "[ ] " + label;
    if (toggledOn) {
        text = "[x] " + label;
    }
    tui.drawString(text, fg, renderBg, position.x, position.y);
}

void ToggleButton::onHover(bool hovered) {
    isHovered = hovered;
}

void ToggleButton::onInteract(pressedKey key) {
    if (key == ENTER) {
        toggledOn = !toggledOn;
    }
}
