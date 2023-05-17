/**
 * ui.h
 * ------
 * Contains various functions to draw common user interface elements,
 * like text boxes, buttons, etc. The goal is to be able to define the UI
 * seamlessly in the drawPanel function, so we get easy to read code.
 * This part should really be implemented as simply as possible: it's easy
 * to mess up UI and create complex garbage.
 */
#ifndef PROJET_PASTEQUE_UI_H
#define PROJET_PASTEQUE_UI_H

#include "panel.h"
#include "colors.h"

#define TEXT_INPUT_MAX 128

typedef struct {
    int inactiveColorId;
    int activeColorId;
    int toggledInactiveColorId;
    int toggledActiveColorId;
} ToggleOptionStyle;

typedef struct {
    // Set inside drawPanel
    bool initialized;
    Panel* panel;
    int x;
    int y;
    int width;
    char* text;
    int interactionIndex;
    ToggleOptionStyle style;
    // Set outside drawPanel
    bool toggled;
} ToggleOption;

typedef struct {
    int inactiveColorId;
    int activeColorId;
    int writingColorId;
} TextInputStyle;

typedef struct {
    // Set inside drawPanel
    bool initialized;
    Panel* panel;
    int x;
    int y;
    int width;
    int interactionIndex;
    int maxLength;
    TextInputStyle style;
    // Set outside drawPanel
    char inputText[TEXT_INPUT_MAX];
    bool isWriting;
} TextInput;

typedef struct {
    int selectedIndex;
    bool focused;
} UIState;

typedef enum {
    ND_HORIZONTAL,
    ND_VERTICAL
} NavigationDirection;

typedef struct {
    int startIndex;
    int endIndex;
    NavigationDirection direction;
} UINavBlock;

static ToggleOptionStyle toggleStyleDefault = {PASTEQUE_COLOR_WHITE,
                                               PASTEQUE_COLOR_LIGHT_BLUE_BG,
                                               PASTEQUE_COLOR_BLACK,
                                               PASTEQUE_COLOR_TURQUOISE_BG};

static ToggleOptionStyle toggleStyleButton = {PASTEQUE_COLOR_WHITE,
                                              PASTEQUE_COLOR_LIGHT_BLUE_BG,
                                              PASTEQUE_COLOR_LIGHT_BLUE_BG,
                                              PASTEQUE_COLOR_LIGHT_BLUE_BG};

static TextInputStyle textInputStyleDefault = {PASTEQUE_COLOR_BLACK,
                                               PASTEQUE_COLOR_LIGHT_BLUE_BG,
                                               PASTEQUE_COLOR_TURQUOISE_BG};

void uiDrawToggleOption(Panel* panel, UIState* state, ToggleOption* option, int x, int y, int width, char* text,
                        int interactionIndex, ToggleOptionStyle style);
// True when toggled
bool uiHandleToggleOptionEvent(UIState* state, ToggleOption* option, Event* event);

void uiDrawTextInput(Panel* panel, UIState* state, TextInput* input, int x, int y, int width, int maxLength,
                     int interactionIndex,
                     TextInputStyle style);

bool uiHandleTextInputEvent(UIState* state, TextInput* input, Event* event);

// End is inclusive!
void uiKeyboardNav(UIState* state, Event* event, UINavBlock blocks[], int nBlocks);

#endif //PROJET_PASTEQUE_UI_H
