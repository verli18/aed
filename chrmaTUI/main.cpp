#include "chrmaTUI.hpp"
#include "elements.hpp"
#include <unistd.h>

#define TEXT {4, 99, 116, 255}
#define TEXT_HIGHLIGHT {4, 191, 181, 255}
#define TEXT_BACKGROUND {4, 33, 49, 255}
#define TEXT_BACKGROUND_HIGHLIGHT {4, 99, 116, 255}
#define BACKGROUND {4, 14, 22, 255}
#define TRANSPARENT {0, 0, 0, 0}

#define WARNING {222, 102, 4, 255}
#define WARNING_HIGHLIGHT {255, 217, 82, 255}
#define WARNING_BACKGROUND {222, 102, 4, 0}
#define WARNING_BACKGROUND_HIGHLIGHT {4, 14, 22, 255}

std::string capturedText = "";
void drawAfterCaptureEnd(element& el, TUImanager& tui) {
    capturedText = "Captured: " + static_cast<InputBar&>(el).text;
}

int main() {
    TUImanager tui;

    container mainContainer({0,0}, {30,17}, {TEXT, TRANSPARENT, TEXT_HIGHLIGHT, TRANSPARENT}, "Main Menu");
    tui.containerID = &mainContainer;
    mainContainer.tui = &tui;

    // Centered popup: x uses cols, y uses rows
    container popup({tui.cols/2 - 20, tui.rows/2 - 11+3}, {40,8}, {WARNING, WARNING_BACKGROUND, WARNING_HIGHLIGHT, WARNING_BACKGROUND_HIGHLIGHT}, "⚠ Warning!");
    
    ToggleButton button1("Option 1", {5, 2}, 20, 1);
    ToggleButton button2("Option 2", {5, 3}, 20, 1);
    ToggleButton button3("Option 3", {5, 4}, 20, 1);
    InputBar input1("Name:", {5, 6}, 20, 2);
    Slider slider1(0.0f, 100.0f, 50.0f, 1.0f, {3, 9}, 24, 1);

    bool continuePressed = false;
    Button openPopup("test popup", {3, 12}, 3, 3);
    Button closePopup("Yes", {tui.cols/2 - 18, tui.rows/2 - 7 + 3}, 3, 3);
    Button secondPopup("No", {tui.cols/2 - 6, tui.rows/2 - 7 + 3}, 3, 3);

    input1.setCaptureEndHandler(drawAfterCaptureEnd);
    // Apply palette using unified standardStyle
    standardStyle buttonStyle{TEXT, TEXT_BACKGROUND, TEXT_HIGHLIGHT, TEXT_BACKGROUND_HIGHLIGHT};
    standardStyle inputStyle{TEXT, TEXT_BACKGROUND, TEXT_HIGHLIGHT, TEXT_BACKGROUND_HIGHLIGHT};
    standardStyle sliderStyle{TEXT, TRANSPARENT, TEXT_HIGHLIGHT, TRANSPARENT};

    standardStyle warnStyle{WARNING, WARNING_BACKGROUND, WARNING_HIGHLIGHT, WARNING_BACKGROUND_HIGHLIGHT};

    button1.setStyle(buttonStyle);
    button2.setStyle(buttonStyle);
    button3.setStyle(buttonStyle);
    input1.setStyle(inputStyle);
    slider1.setStyle(sliderStyle);
    openPopup.setStyle(sliderStyle);
    closePopup.setStyle(warnStyle);
    secondPopup.setStyle(warnStyle);

    openPopup.onClickHandler = [&continuePressed, &popup](element& el, TUImanager& tui){
        continuePressed = true;
        tui.focusContainer(&popup);
    };

    closePopup.onClickHandler = [&continuePressed, &mainContainer](element& el, TUImanager& tui){
        continuePressed = false;
        tui.focusContainer(&mainContainer);
    };

    mainContainer.addElement(&button1);
    mainContainer.addElement(&button2);
    mainContainer.addElement(&button3);
    mainContainer.addElement(&input1);
    mainContainer.addElement(&slider1);
    mainContainer.addElement(&openPopup);

    popup.addElement(&closePopup);
    popup.addElement(&secondPopup);

    if (!mainContainer.elements.empty()) {
        mainContainer.elements[mainContainer.focusedIndex]->notifyHover(tui, true);
    }

    // Main loop
    while (!tui.pollInput()) {
        // Clear the buffer for the new frame
        tui.clearScreen(BACKGROUND);
        tui.drawBox(3, 1, 24, 8, {4, 191, 181, 255}, {0, 0, 0, 0}, {30, 30, 30, 0});

        tui.drawString(capturedText, TEXT, TRANSPARENT, 30, 7);

        mainContainer.render(tui);

        if (continuePressed) {
            popup.render(tui);
            tui.drawString("I am a pop-up!", WARNING, WARNING_BACKGROUND, tui.cols/2 - 18, tui.rows/2 -9 + 3);
        }
        // Run deferred end-of-frame draws (e.g., capture-end handlers)
        tui.runEndOfFrame();

        // Render the TUI
        tui.render();
    }

    return 0;
}