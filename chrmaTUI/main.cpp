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
    (void)tui;
    capturedText = "Captured: " + static_cast<InputBar&>(el).text;
}

int main() {
    TUImanager tui;

    container mainContainer({0,0}, {tui.cols/2,tui.rows}, {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND}, "Main Menu");
    // Set default element style and enable inheritance
    mainContainer.setDefaultElementStyle({TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND});
    mainContainer.setInheritStyle(true);

    tui.containerID = &mainContainer;
    mainContainer.tui = &tui;

    // Centered popup: use percent position
    container popup({0,0}, {20,8}, {WARNING, WARNING_BACKGROUND, WARNING_HIGHLIGHT, WARNING_BACKGROUND_HIGHLIGHT}, "⚠ Warning!");
    popup.position.x = tui.cols * 0.5f - 40;
    popup.position.y = tui.rows * 0.5f - 10;
    popup.setZIndex(10);
    popup.setDefaultElementStyle({WARNING, WARNING_BACKGROUND, WARNING_HIGHLIGHT, WARNING_BACKGROUND_HIGHLIGHT});
    popup.setInheritStyle(true);

    ToggleButton button1("Option 1", {0, 0}, 20, 1);
    button1.setPercentPosition(20, 10);

    ToggleButton button2("Option 2", {0, 0}, 20, 1);
    button2.setPercentPosition(20, 20);

    ToggleButton button3("Option 3", {0, 0}, 20, 1);
    button3.setPercentPosition(20, 30);

    InputBar input1("Name", {0, 0}, 20, 2);
    input1.setPercentPosition(20, 45);

    Slider slider1(0.0f, 100.0f, 50.0f, 1.0f, {0, 0}, 24, 1);
    slider1.setPercentPosition(10, 60);
    slider1.setPercentW(80);

    Selector selector1("Theme", {"Dark", "Light", "Blue", "Green", "Purple"}, {0, 0}, 20, 3);
    selector1.setPercentPosition(60, 15);

    DropdownMenu dropdown1("Language", {"English", "Spanish", "French", "German", "Portuguese", "Italian", "Japanese", "Chinese", "Korean", "Finnish", "Russian", "Arabic", "Turkish", "Vietnamese"}, {0, 0}, 25, 3, 7);
    dropdown1.setPercentPosition(60, 35);

    Text pctLabel("20% x, 50% y", {0, 0});
    pctLabel.setPercentPosition(20, 50);
    
    // Multi-line text example
    std::string multiLineText = "This is a multi-line text example. It should wrap automatically based on the specified width. This demonstrates how we can display longer texts in our TUI library.";
    Text multiLineLabel(multiLineText, {0, 0}, true, 30);  // Enable multi-line with 30 character width
    multiLineLabel.setPercentPosition(50, 75);

    bool continuePressed = false;
    Button openPopup("test popup", {0, 0}, 3, 3);
    openPopup.setPercentPosition(10, 80);

    Button closePopup("Yes", {0, 0}, 3, 3);
    closePopup.setPercentPosition(20, 50);

    Button secondPopup("No", {0, 0}, 3, 3);
    secondPopup.setPercentPosition(60, 50);

    Text popupText("I am a pop-up!", {0, 0});
    popupText.setPercentPosition(10, 20);
    popupText.setAnchors(element::AnchorX::Center, element::AnchorY::Top);

    input1.setCaptureEndHandler(drawAfterCaptureEnd);

    openPopup.onClickHandler = [&continuePressed, &popup](element& /*el*/, TUImanager& tui){
        (void)tui;
        continuePressed = true;
        tui.focusContainer(&popup);
    };

    closePopup.onClickHandler = [&continuePressed, &mainContainer](element& /*el*/, TUImanager& tui){
        (void)tui;
        continuePressed = false;
        tui.focusContainer(&mainContainer);
    };

    mainContainer.addElement(&button1);
    mainContainer.addElement(&button2);
    mainContainer.addElement(&button3);
    mainContainer.addElement(&input1);
    mainContainer.addElement(&slider1);
    mainContainer.addElement(&selector1);
    mainContainer.addElement(&dropdown1);
    mainContainer.addElement(&openPopup);
    mainContainer.addElement(&multiLineLabel);

    // Create a secondary container below the main one to host a MultiLineInput
    int secondHeight = 30;
    container bottomContainer({tui.cols/2, 0}, {tui.cols/2, tui.rows}, {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND}, "Notes");
    bottomContainer.setDefaultElementStyle({TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND});
    bottomContainer.setInheritStyle(true);

    // Add a MultiLineInput to the bottom container
    MultiLineInput notes("Notes", {1,1}, 30, secondHeight - 2);
    notes.setRelativeToInterior(true);
    // Make the notes fill the container interior
    notes.setPercentPosition(0, 0);
    notes.setPercentW(100);
    notes.setPercentH(100);
    bottomContainer.addElement(&notes);

    // Link navigation: from main down to bottom, and bottom up to main
    mainContainer.setRight(&bottomContainer);
    bottomContainer.setLeft(&mainContainer);

    // Ensure bottomContainer is rendered after main so it appears below

    popup.addElement(&closePopup);
    popup.addElement(&secondPopup);
    popup.addElement(&popupText);

    if (!mainContainer.elements.empty()) {
        mainContainer.elements[mainContainer.focusedIndex]->notifyHover(tui, true);
    }

    // Prime background once; avoid clearing every frame to keep dirty rects effective
    tui.clearScreen(BACKGROUND);

    // Main loop
    while (true) {
        // If nothing is dirty and no immediate input, wait up to 16ms (~60 FPS) for input
        if (!tui.hasDirty()) {
            bool hasInput = tui.waitForInput(16);
            if (!hasInput) {
                // nothing happened; continue loop without render
                continue;
            }
        }
        if (tui.pollInput()) break;

        // Example dynamic draws (could be conditional based on state)
        tui.drawBox(3, 1, 24, 8, {4, 191, 181, 255}, {0, 0, 0, 0}, {30, 30, 30, 0});
        tui.drawString(capturedText, TEXT, TRANSPARENT, 30, 7);

        mainContainer.render(tui);

        if (continuePressed) {
            popup.render(tui);
        }
        // Render bottom notes container below main
        bottomContainer.render(tui);
        tui.runEndOfFrame();
        if (tui.hasDirty()) {
            tui.render();
        }
    }

    return 0;
}