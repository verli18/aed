#include "chrmaTUI.hpp"
#include "elements.hpp"
#include <unistd.h>

void drawAfterCaptureEnd(element& el, TUImanager& tui) {
    std::string capturedText = "Captured: " + static_cast<InputBar&>(el).text;
    tui.drawString(capturedText, {255, 255, 0, 255}, {0, 0, 0, 255}, 5, 10);
}

int main() {
    TUImanager tui;
    tui.userState = NAVIGATING;

    container mainContainer;
    tui.containerID = &mainContainer;
    mainContainer.tui = &tui;

    ToggleButton button1("Option 1", {5, 2}, 20, 1);
    ToggleButton button2("Option 2", {5, 3}, 20, 1);
    ToggleButton button3("Option 3", {5, 4}, 20, 1);
    InputBar input1("Name:", {5, 6}, 20, 2);

    input1.setCaptureEndHandler(drawAfterCaptureEnd);
    mainContainer.addElement(&button1);
    mainContainer.addElement(&button2);
    mainContainer.addElement(&button3);
    mainContainer.addElement(&input1);


    mainContainer.focusedIndex = 0;
    if (!mainContainer.elements.empty()) {
        mainContainer.elements[mainContainer.focusedIndex]->notifyHover(tui, true);
    }

    // Main loop
    while (!tui.pollInput()) {
        // Clear the buffer for the new frame
        tui.clearScreen({0,0,0,255});
        // Render all elements in the container
        for (element* el : mainContainer.elements) {
            el->render(tui);
        }

        // Run deferred end-of-frame draws (e.g., capture-end handlers)
        tui.runEndOfFrame();

        // Render the TUI
        tui.render();
    }

    return 0;
}