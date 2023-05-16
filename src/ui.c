#include "ui.h"
#include "colors.h"
#include <string.h>

bool mouseIntersect(Panel* panel, Event* event, int x, int y, int width, int height) {
    int minX = x + panel->x;
    int maxX = minX + width;
    int minY = y + panel->y;
    int maxY = minY + height;

    // Highlight the button when the cursor goes over it.
    return (event->mouseEvent.x >= minX && event->mouseEvent.x < maxX
            && event->mouseEvent.y >= minY && event->mouseEvent.y < maxY);
}

void drawToggleOption(Panel* panel, UIState* state, ToggleOption* option, int x, int y, int width,
                      char* text, int interactionIndex, ToggleOptionStyle style) {
    if (!option->initialized) {
        option->x = x;
        option->y = y;
        option->width = width;
        option->text = text;
        option->interactionIndex = interactionIndex;
        option->style = style;
        option->panel = panel;
        option->initialized = true;
    }

    ColorId color;
    if (state->focused && state->selectedIndex == option->interactionIndex) {
        color = option->toggled ? option->style.toggledActiveColorId : option->style.activeColorId;
    } else {
        color = option->toggled ? option->style.toggledInactiveColorId : option->style.inactiveColorId;
    }

    panelDrawLine(panel, option->x, option->y, option->width, ' ', color);
    int length = (int) strlen(option->text);
    int textX = (option->width - length) / 2;
    if (textX < 0) { textX = 0; }

    panelDrawText(panel, option->x + textX, option->y, option->text, color);
}

bool handleToggleOptionEvent(UIState* state, ToggleOption* option, Event* event) {
    if (!state->focused) {
        return false;
    }

    if (event->code == KEY_MOUSE) {
        // Highlight the button when the cursor goes over it.
        if (mouseIntersect(option->panel, event, option->x, option->y, option->width, 1)) {
            state->selectedIndex = option->interactionIndex;

            // Activate the toggle/button on left click or right click
            if ((event->mouseEvent.bstate & (BUTTON1_RELEASED | BUTTON3_RELEASED)) != 0) {
                return true;
            }
        }
    } else if (state->selectedIndex == option->interactionIndex
               && (event->code == KEY_SPACE || event->code == KEY_RETURN)) {
        // Activate the toggle/button when it is selected when pressing space or enter
        return true;
    }
    return false;
}

void drawTextInput(Panel* panel, UIState* state, TextInput* input, int x, int y, int width, int maxLength,
                   int interactionIndex, TextInputStyle style) {
    if (!input->initialized) {
        input->x = x;
        input->y = y;
        input->width = width;
        input->maxLength = maxLength;
        if (maxLength >= TEXT_INPUT_MAX) {
            input->maxLength = TEXT_INPUT_MAX - 1;
        }
        input->interactionIndex = interactionIndex;
        input->style = style;
        input->panel = panel;
        input->initialized = true;
    }

    ColorId color;
    if (state->focused && state->selectedIndex == input->interactionIndex) {
        color = input->isWriting ? style.writingColorId : style.activeColorId;
    } else {
        color = style.inactiveColorId;
    }

    panelDrawLine(panel, input->x, input->y, input->width, ' ', color);
    panelDrawText(panel, input->x, input->y, input->inputText, color);
}

bool handleTextInputEvent(UIState* state, TextInput* input, Event* event) {
    if (input->isWriting) {
        if (event->code == KEY_RETURN) {
            input->isWriting = false;
        } else {
            int length = (int)strlen(input->inputText);
            if (event->code == KEY_BACKSPACE) {
                if (length > 0) {
                    input->inputText[length-1] = '\0';
                }
            } else if (length != input->maxLength && event->code < 256) {
                // 256+ characters seem to only be constants from ncurses' keypad, ignore those.
                // TODO: Fix bug when reaching max length with 2-chars+ characters.
                input->inputText[length] = (char)event->code;
                input->inputText[length+1] = '\0';
            }
        }
        return true;
    } else if (state->focused && input->interactionIndex == state->selectedIndex) {
        if (event->code == KEY_RETURN) {
            input->isWriting = true;
            return true;
        }
    }
    return false;
}
