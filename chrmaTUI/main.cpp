#include "chrmaTUI.hpp"
#include "elements.hpp"
#include <unistd.h>

int main() {
    TUImanager tui;
    tui.userState = NAVIGATING;

    container mainContainer;
    tui.containerID = &mainContainer;

    ToggleButton button1("Option 1", {5, 2}, 20, 1);
    ToggleButton button2("Option 2", {5, 3}, 20, 1);
    ToggleButton button3("Option 3", {5, 4}, 20, 1);

    mainContainer.addElement(&button1);
    mainContainer.addElement(&button2);
    mainContainer.addElement(&button3);

    mainContainer.focusedIndex = 0;
    if (!mainContainer.elements.empty()) {
        mainContainer.elements[mainContainer.focusedIndex]->onHover(true);
    }

    // Main loop
    while (!tui.windowShouldClose()) {
        // Clear the buffer for the new frame
        for (int y = 0; y < tui.rows; ++y) {
            for (int x = 0; x < tui.cols; ++x) {
                tui.screenBuffer[y][x] = {' ', {0,0,0,0}, {20, 20, 20, 255}};
            }
        }

        // Render all elements in the container
        for (element* el : mainContainer.elements) {
            el->render(tui);
        }

        // Render the TUI
        tui.render();

        // Small delay
        usleep(16000); // ~60 fps
    }

    return 0;
}