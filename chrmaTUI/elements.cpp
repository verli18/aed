#include "elements.hpp"
#include <cctype>

// --- Button Implementation ---
Button::Button(const std::string& lbl, point pos, int w, int h) : label(lbl) {
    position = pos;
    size = {w, h};
    fg = {255, 255, 255, 255};
    bg = {0, 0, 255, 255};
    isHovered = false;
}

void Button::render(TUImanager& tui) {
    color renderBg = isHovered ? bg : color{static_cast<uint8_t>(bg.r / 2), static_cast<uint8_t>(bg.g / 2), static_cast<uint8_t>(bg.b / 2), bg.a};
    tui.drawString(label, fg, renderBg, position.x, position.y);
}

void Button::onHover(bool hovered) {
    isHovered = hovered;
}

void Button::onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) {
    if (key == ENTER) {
    notifyClick(tui);
    if (onClick) onClick();
    }
}

// --- ToggleButton Implementation ---
ToggleButton::ToggleButton(const std::string& lbl, point pos, int w, int h) : label(lbl) {
    position = pos;
    size = {w, h};
    fg = {255, 255, 255, 255};
    bg = {80, 80, 80, 255};
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

void ToggleButton::onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) {
    if (key == ENTER) {
        toggledOn = !toggledOn;
    notifyClick(tui);
    if (onClickHandler) onClickHandler(*this, tui);
    }
}

// --- InputBar Implementation ---
InputBar::InputBar(const std::string& lbl, point pos, int w, int h) : label(lbl) {
    position = pos;
    size = {w, h};
    fg = {255, 255, 255, 255};
    bg = {50, 50, 50, 255};
    isHovered = false;
}

void InputBar::render(TUImanager& tui) {
    color renderBg = isHovered ? color{70, 70, 70, 255} : bg;
    tui.drawString(label, fg, {0,0,0,0}, position.x, position.y);

    std::string renderText = text;
    if (tui.userState == CAPTURE && isHovered) {
        renderText += '_'; // Simple cursor
    }

    for (int i = 0; i < size.x; ++i) {
        tui.drawCharacter({' ', fg, renderBg}, position.x + i, position.y + 1);
    }
    tui.drawString(renderText, fg, renderBg, position.x + 1, position.y + 1);
}

void InputBar::onHover(bool hovered) {
    isHovered = hovered;
}

void InputBar::onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) {
    if (key == ENTER) {
        userState = NAVIGATING;
        notifyCaptureEnd(tui);
    } else if (key == BACKSPACE) {
        if (!text.empty()) {
            text.pop_back();
        }
    } else if (std::isprint(static_cast<unsigned char>(c))) {
        if (text.length() < size.x - 2) { // Leave space for cursor and padding
            text += c;
        }
    }
}

