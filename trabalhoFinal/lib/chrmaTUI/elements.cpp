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
    tui.drawBox(renderPos.x, renderPos.y, w, h, useFg, useBg, useBg);

    // Compute interior width and truncate label if needed to avoid drawing over borders
    int interior = w - 2;
    std::string text = label;
    if (interior > 0 && (int)text.size() > interior) {
        text = text.substr(0, interior);
    }

    // Vertically center text within the box
    int textY = renderPos.y + (h / 2);
    int textX = renderPos.x + 1; // left padding inside the box
    tui.drawString(text, useFg, useBg, textX, textY);
}

void Button::onHover(bool hovered) {
    isHovered = hovered;
}

void Button::onInteract(pressedKey key, char /*c*/, uint8_t& /*userState*/, TUImanager& tui) {
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
    color useFg = isHovered ? style.fgHi : style.fg;
    color useBg = isHovered ? style.bgHi : style.bg;
    
    int totalWidth = std::max(size.x, static_cast<int>(label.size()) + 8);
    int boxHeight = 3;
    
    tui.drawBox(renderPos.x, renderPos.y, totalWidth, boxHeight, useFg, useBg, useBg);
    
    std::string toggleText;
    color toggleFg = useFg;
    
    if (toggledOn) {
        toggleText = "[X]";
        toggleFg = isHovered ? style.fgHi : style.fg;
    } else {
        toggleText = "[ ]";
        toggleFg = useFg;
    }
    
    tui.drawString(toggleText, toggleFg, useBg, renderPos.x + 1, renderPos.y + 1);
    tui.drawString(label, useFg, useBg, renderPos.x + 6, renderPos.y + 1);
}

void ToggleButton::onHover(bool hovered) {
    isHovered = hovered;
}

void ToggleButton::onInteract(pressedKey key, char /*c*/, uint8_t& /*userState*/, TUImanager& tui) {
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
    color useFg = isHovered ? style.fgHi : style.fg;
    color useBg = isHovered ? style.bgHi : style.bg;
    
    int totalWidth = std::max(size.x, static_cast<int>(label.size()) + 4);
    int boxHeight = 3;
    
    color borderColor = useFg;
    color fieldBg = useBg;
    
    tui.drawBox(renderPos.x, renderPos.y, totalWidth, boxHeight, borderColor, useBg, useBg);
    
    std::string labelText = "{" + label + "}";
    tui.drawString(labelText, useFg, {0,0,0,0}, renderPos.x + 1, renderPos.y);
    
    std::string renderText = text;
    bool showCursor = (tui.userState == CAPTURE && isHovered);
    
    int fieldWidth = totalWidth - 2; // Account for borders
    for (int i = 0; i < fieldWidth; ++i) {
        characterSpace cs{};
        cs.character = ' ';
        cs.colorForeground = useFg;
        cs.colorBackground = fieldBg;
        cs.z = 0;
        cs.utf8.clear();
        tui.drawCharacter(cs, renderPos.x + 1 + i, renderPos.y + 1);
    }
    
    // Draw the text content
    if (!renderText.empty()) {
        // Truncate text if too long
        int maxTextWidth = fieldWidth - 2;
        if ((int)renderText.length() > maxTextWidth) {
            renderText = renderText.substr(0, maxTextWidth - 3) + "...";
        }
        tui.drawString(renderText, useFg, fieldBg, renderPos.x + 2, renderPos.y + 1);
    }
    
    // Draw cursor using underscore character (more consistent)
    if (showCursor) {
        int cursorX = renderPos.x + 2 + std::min((int)text.length(), fieldWidth - 3);
        tui.drawString("_", useFg, fieldBg, cursorX, renderPos.y + 1);
    }
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
        if (text.length() < static_cast<size_t>(std::max(0, size.x - 2))) { 
            text += c;
        }
    }
}

// --- Slider Implementation ---
Slider::Slider(float min, float max, float initialValue, float step, point pos, int w, int h)
    : value(initialValue), step(step), minValue(min), maxValue(max) {
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
    tui.drawBox(renderPos.x, renderPos.y, size.x, 3, useFg, useBg, style.bg);
    
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
        tui.drawString(barChar, useFg, useBg, renderPos.x + 1 + i, renderPos.y + 1);
    }
    
    char buf[64];
    snprintf(buf, sizeof(buf), "%g", value);
    std::string valueStr = buf;
    tui.drawString("{", useFg, {0,0,0,0}, renderPos.x+1, renderPos.y);
    tui.drawString(valueStr, useFg, {0,0,0,0}, renderPos.x+2, renderPos.y);
    tui.drawString("}", useFg, {0,0,0,0}, renderPos.x+2+valueStr.length(), renderPos.y);
}

void Slider::onHover(bool hovered) {
    isHovered = hovered;
}

void Slider::onInteract(pressedKey key, char /*c*/, uint8_t& userState, TUImanager& tui) {
    (void)tui;
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

// --- Selector Implementation ---
Selector::Selector(const std::string& lbl, const std::vector<std::string>& opts, point pos, int w, int h)
    : options(opts), selectedIndex(0), label(lbl) {
    position = pos;
    size = {w, h};
    style.fg = {255, 255, 255, 255};
    style.bg = {80, 80, 80, 255};
    style.fgHi = {255, 255, 255, 255};
    style.bgHi = {100, 100, 100, 255};
    isHovered = false;
    if (options.empty()) {
        options.push_back("No options");
    }
    if (selectedIndex >= (int)options.size()) {
        selectedIndex = 0;
    }
}

void Selector::render(TUImanager& tui) {
    color useFg = isHovered ? style.fgHi : style.fg;
    color useBg = isHovered ? style.bgHi : style.bg;
    
    // Draw box around the selector (3 rows tall)
    tui.drawBox(renderPos.x, renderPos.y, size.x, 3, useFg, useBg, style.bg);
    
    // Draw label with curly brackets at the top
    std::string labelText = "{" + label + "}";
    tui.drawString(labelText, useFg, {0,0,0,0}, renderPos.x + 1, renderPos.y);
    
    // Draw the selected option in the middle row
    std::string selectedText = options[selectedIndex];
    int maxWidth = size.x - 4; // Leave space for borders and arrows
    if ((int)selectedText.length() > maxWidth) {
        selectedText = selectedText.substr(0, maxWidth - 3) + "...";
    }
    
    // Draw arrows and selected option
    tui.drawString("◀", useFg, useBg, renderPos.x + 1, renderPos.y + 1);
    tui.drawString(selectedText, useFg, useBg, renderPos.x + 3, renderPos.y + 1);
    tui.drawString("▶", useFg, useBg, renderPos.x + size.x - 2, renderPos.y + 1);
    
    // Show index indicator at bottom
    char indexStr[32];
    snprintf(indexStr, sizeof(indexStr), "%d/%d", selectedIndex + 1, (int)options.size());
    tui.drawString(indexStr, useFg, {0,0,0,0}, renderPos.x + 1, renderPos.y + 2);
}

void Selector::onHover(bool hovered) {
    isHovered = hovered;
}

void Selector::onInteract(pressedKey key, char /*c*/, uint8_t& userState, TUImanager& tui) {
    (void)tui;
    if (key == ENTER) {
        userState = NAVIGATING;
        notifyCaptureEnd(tui);
    } else if (key == LEFT) {
        selectedIndex--;
        if (selectedIndex < 0) {
            selectedIndex = (int)options.size() - 1;
        }
    } else if (key == RIGHT) {
        selectedIndex++;
        if (selectedIndex >= (int)options.size()) {
            selectedIndex = 0;
        }
    }
}

std::string Selector::getSelectedOption() const {
    if (selectedIndex >= 0 && selectedIndex < (int)options.size()) {
        return options[selectedIndex];
    }
    return "";
}

// --- DropdownMenu Implementation ---
DropdownMenu::DropdownMenu(const std::string& lbl, const std::vector<std::string>& opts, point pos, int w, int h, int maxHeight)
    : options(opts), selectedIndex(0), label(lbl), isOpen(false), maxOpenHeight(maxHeight) {
    position = pos;
    size = {w, h};
    style.fg = {255, 255, 255, 255};
    style.bg = {80, 80, 80, 255};
    style.fgHi = {255, 255, 255, 255};
    style.bgHi = {100, 100, 100, 255};
    isHovered = false;
    if (options.empty()) {
        options.push_back("No options");
    }
    if (selectedIndex >= (int)options.size()) {
        selectedIndex = 0;
    }
}

// --- RadioButton Implementation ---
RadioButton::RadioButton(const std::string& lbl, const std::string& grp, point pos, int w, int h)
    : label(lbl), group(grp) {
    position = pos;
    size = {w, h};
    style.fg = {255,255,255,255};
    style.bg = {50,50,50,255};
    style.fgHi = {255,255,255,255};
    style.bgHi = {80,80,80,255};
    isHovered = false;
}

void RadioButton::render(TUImanager& tui) {
    color useFg = isHovered ? style.fgHi : style.fg;
    color useBg = isHovered ? style.bgHi : style.bg;

    int w = std::max(size.x, static_cast<int>(label.size()) + 4);
    int h = std::max(1, size.y);
    // Draw the label text and circle indicator
    // Indicator: ( ) for unselected, (●) for selected
    std::string indicator = selected ? "(●)" : "( )";
    tui.drawString(indicator, useFg, useBg, renderPos.x, renderPos.y + (h/2));
    tui.drawString(" " + label, useFg, useBg, renderPos.x + 4, renderPos.y + (h/2));
}

void RadioButton::onHover(bool hovered) {
    isHovered = hovered;
}

void RadioButton::onInteract(pressedKey key, char /*c*/, uint8_t& userState, TUImanager& tui) {
    if (key == ENTER) {
        // Deselect siblings in the same container with the same group
        if (parent) {
            for (element* el : parent->elements) {
                RadioButton* rb = dynamic_cast<RadioButton*>(el);
                if (rb && rb != this && rb->group == group) {
                    rb->selected = false;
                }
            }
        }
        selected = true;
        notifyClick(tui);
    }
}

void DropdownMenu::render(TUImanager& tui) {
    color useFg = isHovered ? style.fgHi : style.fg;
    color useBg = isHovered ? style.bgHi : style.bg;
    color selectedBg = {120, 120, 120, 255};
    
    if (!isOpen) {
        // Closed state - draw just the header box (3 rows tall like selector)
        tui.drawBox(renderPos.x, renderPos.y, size.x, 3, useFg, useBg, style.bg);
        
        // Draw label with curly brackets at the top
        std::string labelText = "{" + label + "}";
        tui.drawString(labelText, useFg, {0,0,0,0}, renderPos.x + 1, renderPos.y);
        
        // Draw the selected option in the middle row with dropdown arrow
        std::string selectedText = options[selectedIndex];
        int maxWidth = size.x - 4; // Leave space for borders and arrow
        if ((int)selectedText.length() > maxWidth) {
            selectedText = selectedText.substr(0, maxWidth - 3) + "...";
        }
        
        tui.drawString(selectedText, useFg, useBg, renderPos.x + 1, renderPos.y + 1);
        tui.drawString("▼", useFg, useBg, renderPos.x + size.x - 2, renderPos.y + 1);
    } else {
        // Open state - draw expanded dropdown with all options
        int dropdownHeight = std::min((int)options.size() + 2, maxOpenHeight); // Use maxOpenHeight
        tui.drawBox(renderPos.x, renderPos.y, size.x, dropdownHeight, useFg, useBg, style.bg);
        
        // Draw label with curly brackets at the top
        std::string labelText = "{" + label + "}";
        tui.drawString(labelText, useFg, {0,0,0,0}, renderPos.x + 1, renderPos.y);
        
        // Calculate scrolling parameters
        int maxDisplayOptions = dropdownHeight - 2; // Account for header and borders
        bool needsScrollbar = (int)options.size() > maxDisplayOptions;
        int scrollbarWidth = needsScrollbar ? 1 : 0;
        int contentWidth = size.x - 2 - scrollbarWidth; // Leave space for scrollbar if needed
        
        // Draw up arrow (only if scrollbar is needed, otherwise use full width)
        if (needsScrollbar) {
            tui.drawString("▲", useFg, useBg, renderPos.x + size.x - 2, renderPos.y + 1);
        }
        
        // Calculate scroll position to keep selected item visible
        int startIndex = 0;
        if (needsScrollbar) {
            startIndex = std::max(0, std::min(selectedIndex - maxDisplayOptions/2, (int)options.size() - maxDisplayOptions));
        }
        
        // Draw options
        for (int i = 0; i < maxDisplayOptions && (startIndex + i) < (int)options.size(); ++i) {
            int optionIndex = startIndex + i;
            std::string optionText = options[optionIndex];
            
            // Truncate if too long (accounting for scrollbar space)
            if ((int)optionText.length() > contentWidth - 1) {
                optionText = optionText.substr(0, contentWidth - 4) + "...";
            }
            
            // Use inverted colors for selected option
            color optionBg, optionFg;
            if (optionIndex == selectedIndex) {
                optionBg = style.fgHi; // Use highlight foreground as background
                optionFg = style.bgHi; // Use highlight background as foreground
            } else {
                optionBg = useBg;
                optionFg = useFg;
            }
            
            // Add selection indicator
            std::string displayText = (optionIndex == selectedIndex) ? ">" + optionText : " " + optionText;
            
            tui.drawString(displayText, optionFg, optionBg, renderPos.x + 1, renderPos.y + 1 + i);
        }
        
        // Draw scrollbar if needed
        if (needsScrollbar) {
            int scrollbarX = renderPos.x + size.x - 2;
            int scrollbarStartY = renderPos.y + 2; // Start below the up arrow
            int scrollbarTrackHeight = maxDisplayOptions - 1; // Leave space for bottom arrow
            
            // Calculate scrollbar thumb position and size
            float scrollRatio = (float)startIndex / std::max(1, (int)options.size() - maxDisplayOptions);
            float thumbSize = std::max(1.0f, (float)scrollbarTrackHeight * maxDisplayOptions / options.size());
            int thumbPos = (int)(scrollRatio * (scrollbarTrackHeight - thumbSize));
            
            // Draw scrollbar track
            for (int i = 0; i < scrollbarTrackHeight; ++i) {
                std::string scrollChar;
                if (i >= thumbPos && i < thumbPos + (int)thumbSize) {
                    scrollChar = "█"; // Full block for thumb
                } else {
                    scrollChar = "│"; // Light vertical bar for track
                }
                tui.drawString(scrollChar, useFg, useBg, scrollbarX, scrollbarStartY + i);
            }
            
            // Draw bottom arrow
            tui.drawString("▼", useFg, useBg, scrollbarX, renderPos.y + dropdownHeight - 2);
        }
    }
}

void DropdownMenu::onHover(bool hovered) {
    isHovered = hovered;
}

void DropdownMenu::onInteract(pressedKey key, char /*c*/, uint8_t& userState, TUImanager& tui) {
    (void)tui;
    if (key == ENTER) {
        if (!isOpen) {
            isOpen = true;
        } else {
            isOpen = false;
            userState = NAVIGATING;
            notifyCaptureEnd(tui);
        }
    } else if (isOpen) {
        if (key == UP) {
            selectedIndex--;
            if (selectedIndex < 0) {
                selectedIndex = (int)options.size() - 1;
            }
        } else if (key == DOWN) {
            selectedIndex++;
            if (selectedIndex >= (int)options.size()) {
                selectedIndex = 0;
            }
        }
    } else {
        // When closed, just open on any navigation key
        if (key == UP || key == DOWN || key == LEFT || key == RIGHT) {
            isOpen = true;
        }
    }
}

std::string DropdownMenu::getSelectedOption() const {
    if (selectedIndex >= 0 && selectedIndex < (int)options.size()) {
        return options[selectedIndex];
    }
    return "";
}

// Helper function to measure text width using TUImanager's measureColumns
int Text::measureTextWidth(const std::string& text, TUImanager& tui) const {
    return tui.measureColumns(text);
}

// Helper function to wrap text into lines based on maxWidth
std::vector<std::string> Text::wrapText(const std::string& text, int maxWidth, TUImanager& tui) const {
    std::vector<std::string> lines;
    if (maxWidth <= 0) {
        lines.push_back(text);
        return lines;
    }

    size_t start = 0;
    size_t end = 0;
    
    while (start < text.length()) {
        // Find the end of the current line
        end = start;
        size_t lineEnd = start;
        size_t lastSpace = std::string::npos;
        
        // Process characters until we reach maxWidth or end of text
        while (end < text.length()) {
            // Check for explicit line breaks
            if (text[end] == '\n') {
                lineEnd = end;
                end++;
                break;
            }
            
            // Keep track of spaces for word wrapping
            if (text[end] == ' ') {
                lastSpace = end;
            }
            
            // Temporarily extend the line to check its width
            size_t tempEnd = end + 1;
            std::string tempLine = text.substr(start, tempEnd - start);
            int width = measureTextWidth(tempLine, tui);
            
            // If we've exceeded the max width
            if (width > maxWidth) {
                // If we found a space, break there
                if (lastSpace != std::string::npos && lastSpace > start) {
                    lineEnd = lastSpace;
                    end = lastSpace + 1; // Skip the space
                } else {
                    // No space found, break at the previous character
                    lineEnd = (end > start) ? end : start + 1;
                    if (lineEnd > text.length()) lineEnd = text.length();
                }
                break;
            }
            
            lineEnd = tempEnd;
            end = tempEnd;
        }
        
        // Handle case where we reached the end of text
        if (end >= text.length() && lineEnd == start) {
            lineEnd = text.length();
        }
        
        // Extract the line
        std::string line = text.substr(start, lineEnd - start);
        
        // Remove trailing spaces if we broke at a space
        if (!line.empty() && line.back() == ' ') {
            line.pop_back();
        }
        
        lines.push_back(line);
        start = end;
        
        // Skip any leading spaces on the next line (but not newlines)
        while (start < text.length() && text[start] == ' ') {
            start++;
        }
    }
    
    return lines;
}

// --- Text Implementation ---
void Text::render(TUImanager& tui) {
    // Establish draw position from pre-calculated renderPos
    int drawX = renderPos.x;
    int drawY = renderPos.y;

    // Use element's current style (may have been inherited from container at addElement)
    color fg = style.fg;
    color bg = style.bg;

    // Handle multi-line text
    if (multiLine) {
        int effectiveMaxWidth = maxWidth;
        // If maxWidth is 0, use parent container width or screen width
        if (effectiveMaxWidth <= 0) {
            if (auto* p = getParent()) {
                effectiveMaxWidth = std::max(0, p->size.x - 2); // Account for borders
            } else {
                effectiveMaxWidth = tui.cols - drawX; // To edge of screen
            }
        }
        
        std::vector<std::string> lines = wrapText(content, effectiveMaxWidth, tui);
        
        // Draw each line
        for (size_t i = 0; i < lines.size(); ++i) {
            int lineY = drawY + static_cast<int>(i);
            // Check if we're still within screen bounds
            if (lineY >= 0 && lineY < tui.rows) {
                tui.drawString(lines[i], fg, bg, drawX, lineY);
            }
        }
    } else {
        // Single line behavior (original implementation)
        tui.drawString(content, fg, bg, drawX, drawY);
    }
}

// --- MultiLineInput Implementation ---
MultiLineInput::Wrapped MultiLineInput::wrap(const std::string& t, int maxWidth, TUImanager& tui) const {
    Wrapped w{};
    if (maxWidth <= 0) {
        // Fallback: each character on its own line
        for (size_t i = 0; i < t.size(); ++i) {
            w.lines.push_back(std::string(1, t[i]));
            w.starts.push_back(static_cast<int>(i));
        }
        if (w.lines.empty()) {
            w.lines.push_back("");
            w.starts.push_back(0);
        }
        return w;
    }

    // Split by actual newlines first
    std::vector<std::string> hardLines;
    std::vector<int> hardStarts;
    int pos = 0;
    size_t start = 0;
    while (true) {
        size_t found = t.find('\n', start);
        if (found == std::string::npos) {
            hardLines.push_back(t.substr(start));
            hardStarts.push_back(pos + static_cast<int>(start));
            break;
        } else {
            hardLines.push_back(t.substr(start, found - start));
            hardStarts.push_back(pos + static_cast<int>(start));
            start = found + 1;
        }
    }

    // Now wrap each hard line if it's too wide
    for (size_t h = 0; h < hardLines.size(); ++h) {
        const std::string& line = hardLines[h];
        int lineStart = hardStarts[h];
        
        if (line.empty()) {
            w.lines.push_back("");
            w.starts.push_back(lineStart);
            continue;
        }

        int i = 0;
        int n = static_cast<int>(line.size());
        
        while (i < n) {
            int segmentStart = i;
            int lastSpace = -1;
            
            // Find the longest segment that fits
            while (i < n) {
                std::string candidate = line.substr(segmentStart, i - segmentStart + 1);
                int width = tui.measureColumns(candidate);
                
                if (width > maxWidth && i > segmentStart) {
                    // Too wide, break here
                    break;
                }
                
                if (std::isspace(static_cast<unsigned char>(line[i]))) {
                    lastSpace = i;
                }
                i++;
            }
            
            // Decide where to break
            int breakPoint = i;
            if (i < n && lastSpace >= segmentStart) {
                // Break at last space if we have one
                breakPoint = lastSpace;
                i = lastSpace + 1; // Skip the space for next segment
            } else if (i < n) {
                // Force break at current position
                breakPoint = i;
            }
            
            std::string segment = line.substr(segmentStart, breakPoint - segmentStart);
            w.lines.push_back(segment);
            w.starts.push_back(lineStart + segmentStart);
        }
    }
    
    if (w.lines.empty()) {
        w.lines.push_back("");
        w.starts.push_back(0);
    }
    return w;
}

std::pair<int,int> MultiLineInput::caretWrappedPos(const Wrapped& w) const {
    if (w.lines.empty()) return {0, 0};
    
    // Find which wrapped line contains our caret position
    for (size_t i = 0; i < w.starts.size(); ++i) {
        int lineStart = w.starts[i];
        int lineEnd = lineStart + static_cast<int>(w.lines[i].size());
        
        // Handle newlines: if this line ends with text and next line exists,
        // the newline character belongs to this line conceptually
        if (i + 1 < w.starts.size()) {
            int nextStart = w.starts[i + 1];
            if (nextStart > lineEnd) {
                lineEnd = nextStart; // Include the newline character
            }
        }
        
        if (caretIndex <= lineEnd || i == w.starts.size() - 1) {
            int col = caretIndex - lineStart;
            // Clamp column to the actual visible line length
            col = std::min(col, static_cast<int>(w.lines[i].size()));
            col = std::max(col, 0);
            return {static_cast<int>(i), col};
        }
    }
    
    // Fallback: put at end of last line
    int lastLine = static_cast<int>(w.lines.size()) - 1;
    int lastCol = static_cast<int>(w.lines[lastLine].size());
    return {lastLine, lastCol};
}

void MultiLineInput::render(TUImanager& tui) {
    color useFg = isHovered ? style.fgHi : style.fg;
    color useBg = isHovered ? style.bgHi : style.bg;

    int w = std::max(size.x, 6);
    int h = std::max(size.y, 3);

    // Frame
    tui.drawBox(renderPos.x, renderPos.y, w, h, useFg, useBg, useBg);
    // Label on top border
    std::string labelText = "{" + label + "}";
    tui.drawString(labelText, useFg, {0,0,0,0}, renderPos.x + 1, renderPos.y);

    // Content area - check if we need scrollbar
    int innerH = h - 2; // content rows
    Wrapped wrapped = wrap(text, std::max(1, w - 2), tui);
    int totalLines = static_cast<int>(wrapped.lines.size());
    bool needsScrollbar = totalLines > innerH;
    int scrollbarWidth = needsScrollbar ? 1 : 0;
    int contentWidth = w - 2 - scrollbarWidth; // Leave space for scrollbar if needed
    
    // Re-wrap with correct content width if scrollbar is needed
    if (needsScrollbar) {
        wrapped = wrap(text, std::max(1, contentWidth), tui);
    }
    totalLines = static_cast<int>(wrapped.lines.size());
    needsScrollbar = totalLines > innerH; // Re-check after rewrapping
    scrollbarWidth = needsScrollbar ? 1 : 0;
    contentWidth = w - 2 - scrollbarWidth;

    // Ensure caret is visible: compute wrapped position and adjust scrollY
    auto [cLine, cCol] = caretWrappedPos(wrapped);
    if (cLine < scrollY) scrollY = cLine;
    if (cLine >= scrollY + innerH) scrollY = cLine - innerH + 1;
    if (scrollY < 0) scrollY = 0;
    if (scrollY > totalLines - 1) scrollY = std::max(0, totalLines - 1);

    // Draw visible lines
    for (int row = 0; row < innerH; ++row) {
        int ly = scrollY + row;
        // clear line background
        for (int col = 0; col < contentWidth; ++col) {
            characterSpace cs{};
            cs.character = ' ';
            cs.colorForeground = useFg;
            cs.colorBackground = useBg;
            tui.drawCharacter(cs, renderPos.x + 1 + col, renderPos.y + 1 + row);
        }
        if (ly >= 0 && ly < totalLines) {
            std::string line = wrapped.lines[ly];
            // trim to width
            tui.drawString(line, useFg, useBg, renderPos.x + 1, renderPos.y + 1 + row);
        }
    }

    // Draw scrollbar if needed (with triangle markers like dropdown)
    if (needsScrollbar) {
        int scrollbarX = renderPos.x + w - 2; // Position scrollbar at right edge inside border

        // Draw top triangle marker
        tui.drawString("▲", useFg, useBg, scrollbarX, renderPos.y + 1);
        // Draw bottom triangle marker
        tui.drawString("▼", useFg, useBg, scrollbarX, renderPos.y + h - 2);

        // Track starts below the top triangle and ends above the bottom triangle
        int trackStartY = renderPos.y + 2;
        int trackHeight = innerH - 2; // reserve two rows for triangles
        if (trackHeight > 0) {
            // Draw scrollbar track
            for (int row = 0; row < trackHeight; ++row) {
                tui.drawString("│", useFg, useBg, scrollbarX, trackStartY + row);
            }

            // Calculate scrollbar thumb position and size relative to trackHeight
            if (totalLines > innerH) {
                // thumb size proportional to visible lines over total lines
                int thumbSize = std::max(1, static_cast<int>((float)trackHeight * (float)innerH / (float)totalLines));
                float scrollRatio = static_cast<float>(scrollY) / std::max(1, totalLines - innerH);
                int thumbPos = static_cast<int>(scrollRatio * (trackHeight - thumbSize));

                // Draw scrollbar thumb
                for (int i = 0; i < thumbSize; ++i) {
                    if (thumbPos + i < trackHeight) {
                        tui.drawString("█", useFg, useBg, scrollbarX, trackStartY + thumbPos + i);
                    }
                }
            }
        }
    }

    // Draw blinking caret if in capture mode
    if (tui.userState == CAPTURE && isHovered) {
        // Update cursor blink state
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBlinkTime);
        if (elapsed.count() >= 500) { // Blink every 500ms
            cursorVisible = !cursorVisible;
            lastBlinkTime = now;
        }
        
        if (cursorVisible) {
            int caretRow = cLine - scrollY;
            if (caretRow >= 0 && caretRow < innerH) {
                int caretX = renderPos.x + 1 + std::min(std::max(0, cCol), contentWidth - 1);
                int caretY = renderPos.y + 1 + caretRow;
                tui.drawString("█", useFg, useBg, caretX, caretY);
            }
        }
        
        // Show help text at the bottom of the element
        std::string helpText = "{press ESC to exit}";
        int helpY = renderPos.y + h - 1;
        int helpX = renderPos.x + w - static_cast<int>(helpText.length()) - 1;
        if (helpX < renderPos.x + 1) helpX = renderPos.x + 1; // Don't go past left border
        tui.drawString(helpText, useFg, {0,0,0,0}, helpX, helpY);
    }
}

void MultiLineInput::onInteract(pressedKey key, char c, uint8_t& userState, TUImanager& tui) {
    // Exit capture on ESC
    if (key == ESC) {
        userState = NAVIGATING;
        notifyCaptureEnd(tui);
        return;
    }
    // Insert newline on Enter
    if (key == ENTER) {
        text.insert(text.begin() + std::min(caretIndex, (int)text.size()), '\n');
        caretIndex += 1;
        // Reset cursor blink to make it immediately visible after inserting newline
        cursorVisible = true;
        lastBlinkTime = std::chrono::steady_clock::now();
        return;
    }
    // Navigation
    if (key == LEFT) {
        if (caretIndex > 0) caretIndex--;
        // Reset cursor blink to make it immediately visible
        cursorVisible = true;
        lastBlinkTime = std::chrono::steady_clock::now();
        return;
    }
    if (key == RIGHT) {
        if (caretIndex < (int)text.size()) caretIndex++;
        // Reset cursor blink to make it immediately visible
        cursorVisible = true;
        lastBlinkTime = std::chrono::steady_clock::now();
        return;
    }
    if (key == UP || key == DOWN) {
        // Move by wrapped line preserving column
        int w = std::max(size.x, 6);
        int h = std::max(size.y, 3);
        int innerH = h - 2;
        Wrapped tempWrapped = wrap(text, std::max(1, w - 2), tui);
        bool needsScrollbar = static_cast<int>(tempWrapped.lines.size()) > innerH;
        int contentWidth = w - 2 - (needsScrollbar ? 1 : 0);
        
        Wrapped wrapped = wrap(text, std::max(1, contentWidth), tui);
        auto [line, col] = caretWrappedPos(wrapped);
        int target = (key == UP) ? line - 1 : line + 1;
        if (target >= 0 && target < (int)wrapped.lines.size()) {
            int start = wrapped.starts[target];
            int maxCol = static_cast<int>(wrapped.lines[target].size());
            int newCol = std::min(col, maxCol);
            caretIndex = start + newCol;
            
            // Handle newlines: if we're at the end of a line that's followed by a newline,
            // and the next wrapped line starts after a gap, we might be positioning on the newline
            if (target + 1 < (int)wrapped.starts.size()) {
                int currentEnd = start + static_cast<int>(wrapped.lines[target].size());
                int nextStart = wrapped.starts[target + 1];
                if (nextStart > currentEnd && newCol == maxCol && caretIndex < (int)text.size() && text[caretIndex] == '\n') {
                    // We're at the newline position, which is correct
                }
            }
        }
        // Reset cursor blink to make it immediately visible
        cursorVisible = true;
        lastBlinkTime = std::chrono::steady_clock::now();
        return;
    }
    if (key == BACKSPACE) {
        if (caretIndex > 0 && !text.empty()) {
            text.erase(text.begin() + caretIndex - 1);
            caretIndex--;
        }
        // Reset cursor blink to make it immediately visible
        cursorVisible = true;
        lastBlinkTime = std::chrono::steady_clock::now();
        return;
    }
    // Printable characters (raw byte 'c')
    if (c >= 32 && c != 127) {
        text.insert(text.begin() + std::min(caretIndex, (int)text.size()), c);
        caretIndex += 1;
        // Reset cursor blink to make it immediately visible
        cursorVisible = true;
        lastBlinkTime = std::chrono::steady_clock::now();
        return;
    }
}

// --- ListView Implementation ---
ListView::ListView(const std::string& lbl, const std::vector<std::string>& itemList, point pos, int w, int h, int visibleRows)
    : items(itemList), selectedIndex(0), label(lbl), visibleRows(visibleRows), scrollOffset(0) {
    position = pos;
    size = {w, h};
    style.fg = {255, 255, 255, 255};
    style.bg = {80, 80, 80, 255};
    style.fgHi = {255, 255, 255, 255};
    style.bgHi = {100, 100, 100, 255};
    isHovered = false;
    
    if (items.empty()) {
        items.push_back("No items");
    }
    if (selectedIndex >= (int)items.size()) {
        selectedIndex = 0;
    }
}

void ListView::render(TUImanager& tui) {
    color useFg = isHovered ? style.fgHi : style.fg;
    color useBg = isHovered ? style.bgHi : style.bg;
    
    // Use the specified height or calculate from visibleRows
    int totalHeight = std::max(5, size.y); // Minimum height
    int actualVisibleRows = totalHeight - 2; // -2 for label/borders
    
    // Draw container box
    tui.drawBox(renderPos.x, renderPos.y, size.x, totalHeight, useFg, useBg, style.bg);
    
    // Draw label with curly brackets at the top
    std::string labelText = "{" + label + "}";
    tui.drawString(labelText, useFg, {0,0,0,0}, renderPos.x + 1, renderPos.y);
    
    // Calculate scrolling parameters
    bool needsScrollbar = (int)items.size() > actualVisibleRows;
    int scrollbarWidth = needsScrollbar ? 1 : 0;
    int contentWidth = size.x - 2 - scrollbarWidth; // Account for borders and scrollbar
    
    // Adjust scroll offset to keep selected item visible
    if (selectedIndex < scrollOffset) {
        scrollOffset = selectedIndex;
    }
    if (selectedIndex >= scrollOffset + actualVisibleRows) {
        scrollOffset = selectedIndex - actualVisibleRows + 1;
    }
    
    // Clamp scroll offset
    int maxScroll = std::max(0, (int)items.size() - actualVisibleRows);
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
    if (scrollOffset < 0) scrollOffset = 0;
    
    // Draw items
    for (int i = 0; i < actualVisibleRows && (scrollOffset + i) < (int)items.size(); ++i) {
        int itemIndex = scrollOffset + i;
        std::string itemText = items[itemIndex];
        
        // Truncate if too long
        if ((int)itemText.length() > contentWidth - 2) {
            itemText = itemText.substr(0, contentWidth - 5) + "...";
        }
        
        // Use inverted colors for selected item
        color itemBg, itemFg;
        if (itemIndex == selectedIndex) {
            itemBg = style.fgHi; // Use highlight foreground as background
            itemFg = style.bgHi; // Use highlight background as foreground
        } else {
            itemBg = useBg;
            itemFg = useFg;
        }
        
        // Add selection indicator
        std::string displayText = (itemIndex == selectedIndex) ? ">" + itemText : " " + itemText;
        
        // Clear the line background first
        for (int col = 0; col < contentWidth; ++col) {
            characterSpace cs{};
            cs.character = ' ';
            cs.colorForeground = itemFg;
            cs.colorBackground = itemBg;
            tui.drawCharacter(cs, renderPos.x + 1 + col, renderPos.y + 1 + i);
        }
        
        tui.drawString(displayText, itemFg, itemBg, renderPos.x + 1, renderPos.y + 1 + i);
    }
    
    // Draw scrollbar if needed
    if (needsScrollbar) {
        int scrollbarX = renderPos.x + size.x - 2;
        int scrollbarStartY = renderPos.y + 1;
        int scrollbarHeight = actualVisibleRows;
        
        // Draw up arrow
        tui.drawString("▲", useFg, useBg, scrollbarX, scrollbarStartY);
        
        // Calculate scrollbar thumb position and size
        int trackHeight = scrollbarHeight - 2; // Reserve space for arrows
        if (trackHeight > 0) {
            float scrollRatio = (float)scrollOffset / std::max(1, (int)items.size() - actualVisibleRows);
            float thumbSize = std::max(1.0f, (float)trackHeight * actualVisibleRows / items.size());
            int thumbPos = (int)(scrollRatio * (trackHeight - thumbSize));
            
            // Draw scrollbar track
            for (int i = 0; i < trackHeight; ++i) {
                std::string scrollChar;
                if (i >= thumbPos && i < thumbPos + (int)thumbSize) {
                    scrollChar = "█"; // Full block for thumb
                } else {
                    scrollChar = "│"; // Light vertical bar for track
                }
                tui.drawString(scrollChar, useFg, useBg, scrollbarX, scrollbarStartY + 1 + i);
            }
        }
        
        // Draw down arrow
        tui.drawString("▼", useFg, useBg, scrollbarX, scrollbarStartY + scrollbarHeight - 1);
    }
    
    // Show item count at bottom
    char countStr[32];
    snprintf(countStr, sizeof(countStr), "%d/%d", selectedIndex + 1, (int)items.size());
    tui.drawString(countStr, useFg, {0,0,0,0}, renderPos.x + 1, renderPos.y + totalHeight - 1);
}

void ListView::onHover(bool hovered) {
    isHovered = hovered;
}

void ListView::onInteract(pressedKey key, char /*c*/, uint8_t& userState, TUImanager& tui) {
    if (key == ENTER) {
        userState = NAVIGATING;
        notifyClick(tui);
        notifyCaptureEnd(tui);
    } else if (key == UP) {
        selectedIndex--;
        if (selectedIndex < 0) {
            selectedIndex = (int)items.size() - 1;
        }
    } else if (key == DOWN) {
        selectedIndex++;
        if (selectedIndex >= (int)items.size()) {
            selectedIndex = 0;
        }
    } else if (key == ESC) {
        userState = NAVIGATING;
        notifyCaptureEnd(tui);
    }
}

std::string ListView::getSelectedItem() const {
    if (selectedIndex >= 0 && selectedIndex < (int)items.size()) {
        return items[selectedIndex];
    }
    return "";
}

void ListView::setItems(const std::vector<std::string>& newItems) {
    items = newItems;
    if (items.empty()) {
        items.push_back("No items");
    }
    // Reset selection if out of bounds
    if (selectedIndex >= (int)items.size()) {
        selectedIndex = std::max(0, (int)items.size() - 1);
    }
    // Reset scroll
    scrollOffset = 0;
}

// --- RichListView Implementation ---
RichListView::RichListView(const std::string& lbl, const std::vector<RichListItem>& itemList,
                           point pos, int w, int h, int itemHeight)
    : items(itemList), selectedIndex(0), label(lbl), scrollOffset(0), itemHeight(itemHeight) {
    position = pos;
    size = {w, h};
    style.fg = {255, 255, 255, 255};
    style.bg = {80, 80, 80, 255};
    style.fgHi = {255, 255, 255, 255};
    style.bgHi = {100, 100, 100, 255};
    isHovered = false;
    
    if (selectedIndex >= (int)items.size()) {
        selectedIndex = 0;
    }
}

void RichListView::render(TUImanager& tui) {
    color useFg = isHovered ? style.fgHi : style.fg;
    color useBg = isHovered ? style.bgHi : style.bg;
    
    int totalHeight = std::max(5, size.y);
    
    // Draw outer container box
    tui.drawBox(renderPos.x, renderPos.y, size.x, totalHeight, useFg, useBg, style.bg);
    
    // Draw label
    std::string labelText = "{" + label + "}";
    tui.drawString(labelText, useFg, {0,0,0,0}, renderPos.x + 1, renderPos.y);
    
    if (items.empty()) {
        tui.drawString("No items", useFg, useBg, renderPos.x + 2, renderPos.y + 2);
        return;
    }
    
    // Calculate available space for items
    int contentHeight = totalHeight - 2; // -2 for top/bottom borders
    int contentWidth = size.x - 2; // -2 for left/right borders
    
    // Calculate how many items can fit
    int visibleItems = contentHeight / itemHeight;
    if (visibleItems < 1) visibleItems = 1;
    
    // Determine if we need a scrollbar
    bool needsScrollbar = (int)items.size() > visibleItems;
    int scrollbarWidth = needsScrollbar ? 1 : 0;
    int itemWidth = contentWidth - scrollbarWidth - 2; // -2 for item padding
    
    // Adjust scroll offset to keep selected item visible
    if (selectedIndex < scrollOffset) {
        scrollOffset = selectedIndex;
    }
    if (selectedIndex >= scrollOffset + visibleItems) {
        scrollOffset = selectedIndex - visibleItems + 1;
    }
    
    // Clamp scroll offset
    int maxScroll = std::max(0, (int)items.size() - visibleItems);
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
    if (scrollOffset < 0) scrollOffset = 0;
    
    // Draw items
    int currentY = renderPos.y + 1;
    for (int i = 0; i < visibleItems && (scrollOffset + i) < (int)items.size(); ++i) {
        int itemIndex = scrollOffset + i;
        const RichListItem& item = items[itemIndex];
        
        // Determine colors for this item
        color itemFg, itemBg, itemFgHi, itemBgHi;
        if (itemIndex == selectedIndex) {
            // Selected item: use highlight colors
            itemFg = item.theme.fgHi;
            itemBg = item.theme.bgHi;
            itemFgHi = item.theme.fgHi;
            itemBgHi = item.theme.bgHi;
        } else {
            // Normal item: use regular colors
            itemFg = item.theme.fg;
            itemBg = item.theme.bg;
            itemFgHi = item.theme.fgHi;
            itemBgHi = item.theme.bgHi;
        }
        
        // Draw item box (mini-container)
        int itemX = renderPos.x + 2;
        int actualItemHeight = std::min(itemHeight, contentHeight - (currentY - renderPos.y - 1));
        
        if (actualItemHeight < 3) break; // Not enough space for item
        
        tui.drawBox(itemX, currentY, itemWidth, actualItemHeight, itemFg, itemBg, itemBg);
        
        // Draw selection indicator if selected
        if (itemIndex == selectedIndex) {
            tui.drawString(">", itemFgHi, itemBg, itemX - 1, currentY + actualItemHeight / 2);
        }
        
        // Draw multi-line content inside the item box
        int lineY = currentY + 1;
        int maxLines = actualItemHeight - 2; // -2 for top/bottom borders
        int maxLineWidth = itemWidth - 2; // -2 for left/right borders
        
        for (int lineIdx = 0; lineIdx < (int)item.lines.size() && lineIdx < maxLines; ++lineIdx) {
            std::string line = item.lines[lineIdx];
            
            // Truncate if too long
            if ((int)line.length() > maxLineWidth) {
                line = line.substr(0, maxLineWidth - 3) + "...";
            }
            
            tui.drawString(line, itemFg, itemBg, itemX + 1, lineY + lineIdx);
        }
        
        currentY += actualItemHeight;
    }
    
    // Draw scrollbar if needed
    if (needsScrollbar) {
        int scrollbarX = renderPos.x + size.x - 2;
        int scrollbarStartY = renderPos.y + 1;
        int scrollbarHeight = contentHeight;
        
        // Draw up arrow
        tui.drawString("▲", useFg, useBg, scrollbarX, scrollbarStartY);
        
        // Calculate scrollbar thumb
        int trackHeight = scrollbarHeight - 2; // Reserve space for arrows
        if (trackHeight > 0 && items.size() > 0) {
            float scrollRatio = (float)scrollOffset / std::max(1, (int)items.size() - visibleItems);
            float thumbSize = std::max(1.0f, (float)trackHeight * visibleItems / items.size());
            int thumbPos = (int)(scrollRatio * (trackHeight - thumbSize));
            
            // Draw scrollbar track
            for (int i = 0; i < trackHeight; ++i) {
                std::string scrollChar;
                if (i >= thumbPos && i < thumbPos + (int)thumbSize) {
                    scrollChar = "█";
                } else {
                    scrollChar = "│";
                }
                tui.drawString(scrollChar, useFg, useBg, scrollbarX, scrollbarStartY + 1 + i);
            }
        }
        
        // Draw down arrow
        tui.drawString("▼", useFg, useBg, scrollbarX, scrollbarStartY + scrollbarHeight - 1);
    }
    
    // Show item count
    char countStr[32];
    snprintf(countStr, sizeof(countStr), "%d/%d", selectedIndex + 1, (int)items.size());
    tui.drawString(countStr, useFg, {0,0,0,0}, renderPos.x + 1, renderPos.y + totalHeight - 1);
}

void RichListView::onHover(bool hovered) {
    isHovered = hovered;
}

void RichListView::onInteract(pressedKey key, char /*c*/, uint8_t& userState, TUImanager& tui) {
    if (items.empty()) return;
    
    if (key == ENTER) {
        userState = NAVIGATING;
        notifyClick(tui);
        notifyCaptureEnd(tui);
    } else if (key == UP) {
        selectedIndex--;
        if (selectedIndex < 0) {
            selectedIndex = (int)items.size() - 1;
        }
    } else if (key == DOWN) {
        selectedIndex++;
        if (selectedIndex >= (int)items.size()) {
            selectedIndex = 0;
        }
    } else if (key == ESC) {
        userState = NAVIGATING;
        notifyCaptureEnd(tui);
    }
}

const RichListItem* RichListView::getSelectedItem() const {
    if (selectedIndex >= 0 && selectedIndex < (int)items.size()) {
        return &items[selectedIndex];
    }
    return nullptr;
}

void RichListView::setItems(const std::vector<RichListItem>& newItems) {
    items = newItems;
    // Reset selection if out of bounds
    if (selectedIndex >= (int)items.size() && !items.empty()) {
        selectedIndex = (int)items.size() - 1;
    }
    if (items.empty()) {
        selectedIndex = 0;
    }
    // Reset scroll
    scrollOffset = 0;
}