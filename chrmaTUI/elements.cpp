#include "elements.hpp"
#include <cctype>

// --- Button Implementation ---
Button::Button(const std::string& lbl, point pos, int w, int h) : label(lbl) {
    position = pos;
    size = {w, h};
    style.fg = {255, 255, 255, 255};
    style.bg = {0, 0, 255, 255};
    style.fgHi = {255, 255, 255, 255};
    style.bgHi = {0, 100, 255, 255};
    isHovered = false;
}


void Button::render(TUImanager& tui) {
    color useFg = isHovered ? style.fgHi : style.fg;
    color useBg = isHovered ? style.bgHi : style.bg;

    // Ensure a minimum size for a boxed button
    // Auto-size horizontally to fit the label (interior = w-2)
    int w = std::max({3, size.x, static_cast<int>(label.size()) + 2});
    int h = std::max(3, size.y);

    // Draw button box
    tui.drawBox(position.x, position.y, w, h, useFg, useBg, useBg);

    // Compute interior width and truncate label if needed to avoid drawing over borders
    int interior = w - 2;
    std::string text = label;
    if (interior > 0 && (int)text.size() > interior) {
        text = text.substr(0, interior);
    }

    // Vertically center text within the box
    int textY = position.y + (h / 2);
    int textX = position.x + 1; // left padding inside the box
    tui.drawString(text, useFg, useBg, textX, textY);
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
    style.fg = {255, 255, 255, 255};
    style.bg = {80, 80, 80, 255};
    style.fgHi = {0, 0, 0, 255};
    style.bgHi = {200, 200, 200, 255};
    isHovered = false;
}


void ToggleButton::render(TUImanager& tui) {
    color renderBg = isHovered ? style.bgHi : style.bg;
    color renderFg = isHovered ? style.fgHi : style.fg;
    std::string text = "[ ] " + label;
    if (toggledOn) {
        text = "[x] " + label;
    }
    tui.drawString(text, renderFg, renderBg, position.x, position.y);
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
    style.fg = {255, 255, 255, 255};
    style.bg = {50, 50, 50, 255};
    style.fgHi = {255, 255, 255, 255};
    style.bgHi = {70, 70, 70, 255};
    isHovered = false;
}


void InputBar::render(TUImanager& tui) {
    color renderBg = isHovered ? style.bgHi : style.bg;
    color renderFg = isHovered ? style.fgHi : style.fg;
    tui.drawString(label, renderFg, {0,0,0,0}, position.x, position.y);

    std::string renderText = text;
    if (tui.userState == CAPTURE && isHovered) {
        renderText += '_'; // Simple cursor
    }

    for (int i = 0; i < size.x; ++i) {
    tui.drawCharacter({' ', renderFg, renderBg}, position.x + i, position.y + 1);
    }
    tui.drawString(renderText, renderFg, renderBg, position.x + 1, position.y + 1);
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

// --- Slider Implementation ---
Slider::Slider(float min, float max, float initialValue, float step, point pos, int w, int h)
    : minValue(min), maxValue(max), value(initialValue), step(step) {
    position = pos;
    size = {w, h};
    style.fg = {255, 255, 255, 255};
    style.bg = {80, 80, 80, 255};
    style.fgHi = {255, 255, 255, 255};
    style.bgHi = {100, 100, 100, 255};
    isHovered = false;
    if (value < minValue) value = minValue;
    if (value > maxValue) value = maxValue;
}


void Slider::render(TUImanager& tui) {
    color useFg = isHovered ? style.fgHi : style.fg;
    color useBg = isHovered ? style.bgHi : style.bg;
    
    // Draw box around the slider
    tui.drawBox(position.x, position.y, size.x, 3, useFg, useBg, style.bg);
    
    // Calculate the position of the value indicator within the bar
    float normalizedValue = (value - minValue) / (maxValue - minValue);
    int barWidth = size.x - 2; // Account for box borders
    int valuePosition = static_cast<int>(normalizedValue * barWidth);
    
    // Draw the slider bar at y+1 (inside the box)
    for (int i = 0; i < barWidth; ++i) {
        std::string barChar;
        if (i < valuePosition) {
            barChar = "━"; // Heavy horizontal line for filled portion
        } else if (i == valuePosition) {
            barChar = "┃"; // Vertical bar for current value position
        } else {
            barChar = "─"; // Light horizontal line for empty portion
        }
        tui.drawString(barChar, useFg, useBg, position.x + 1 + i, position.y + 1);
    }
    
    char buf[64];
    snprintf(buf, sizeof(buf), "%g", value);
    std::string valueStr = buf;
    tui.drawString("{", useFg, {0,0,0,0}, position.x+1, position.y);
    tui.drawString(valueStr, useFg, {0,0,0,0}, position.x+2, position.y);
    tui.drawString("}", useFg, {0,0,0,0}, position.x+2+valueStr.length(), position.y);
}

void Slider::onHover(bool hovered) {
    isHovered = hovered;
}

void Slider::onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) {
    if (key == ENTER) {
        userState = NAVIGATING;
        notifyCaptureEnd(tui);
    } else if (key == LEFT) {
        value -= step;
    } else if (key == RIGHT) {
        value += step;
    }
    if (value < minValue) value = minValue;
    if (value > maxValue) value = maxValue;
}
