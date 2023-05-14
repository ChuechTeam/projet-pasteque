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

typedef struct {
    int inactiveColorId;
    int activeColorId;
    int toggledColorId;
} ToggleOptionStyle;

typedef struct {
    // Set inside drawPanel
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
    int selectedIndex;
} UIState;

static ToggleOptionStyle toggleStyleMonochrome = {PASTEQUE_COLOR_WHITE,
                                                  PASTEQUE_COLOR_GREY_25_BG,
                                                  PASTEQUE_COLOR_BLACK};

ToggleOption makeToggleOption(Panel* panel, int x, int y, int width, char* text, int interactionIndex, ToggleOptionStyle style);
void drawToggleOption(Panel* panel, const ToggleOption* option, UIState* state);
// True when toggled
bool handleToggleOptionEvent(UIState* state, ToggleOption* option, Event* event);

#endif //PROJET_PASTEQUE_UI_H
