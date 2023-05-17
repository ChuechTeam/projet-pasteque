#include "ui.h"
#include "colors.h"
#include <string.h>

bool mouseIntersect(Panel* panel, Event* event, int x, int y, int width, int height) {
    int minX = x + panel->x;
    int maxX = minX + width;
    int minY = y + panel->y;
    int maxY = minY + height;

    // Highlight the button when the cursor goes over it.
    return event->code == KEY_MOUSE
           && (event->mouseEvent.x >= minX && event->mouseEvent.x < maxX
               && event->mouseEvent.y >= minY && event->mouseEvent.y < maxY);
}

bool mouseClick(Event* event) {
    return event->code == KEY_MOUSE && (event->mouseEvent.bstate & (BUTTON1_RELEASED | BUTTON3_RELEASED)) != 0;
}

void uiDrawToggleOption(Panel* panel, UIState* state, ToggleOption* option, int x, int y, int width,
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

bool uiHandleToggleOptionEvent(UIState* state, ToggleOption* option, Event* event) {
    if (!state->focused) {
        return false;
    }

    // Highlight the button when the cursor goes over it.
    if (mouseIntersect(option->panel, event, option->x, option->y, option->width, 1)) {
        state->selectedIndex = option->interactionIndex;

        // Activate the toggle/button on left click or right click
        if (mouseClick(event) != 0) {
            return true;
        }
    } else if (state->selectedIndex == option->interactionIndex
               && (event->code == KEY_SPACE || event->code == KEY_RETURN)) {
        // Activate the toggle/button when it is selected when pressing space or enter
        return true;
    }
    return false;
}

void uiDrawTextInput(Panel* panel, UIState* state, TextInput* input, int x, int y, int width, int maxLength,
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

bool uiHandleTextInputEvent(UIState* state, TextInput* input, Event* event) {
    bool mouseHover = mouseIntersect(input->panel, event, input->x, input->y, input->width, 1);

    if (input->isWriting) {
        if (event->code == KEY_RETURN || (!mouseHover && mouseClick(event))) {
            // Quit writing when pressing enter OR clicking outside the screen.
            input->isWriting = false;
        } else {
            int length = (int) strlen(input->inputText);
            if (event->code == KEY_BACKSPACE || event->code == 8) { // 8 = ASCII Backspace
                if (length > 0) {
                    input->inputText[length - 1] = '\0';
                }
            } else if (length != input->maxLength && event->code < 256) {
                // 256+ characters seem to only be constants from ncurses' keypad, ignore those.
                // TODO: Fix bug when reaching max length with 2-chars+ characters.
                input->inputText[length] = (char) event->code;
                input->inputText[length + 1] = '\0';
            }
        }
        return true;
    } else if (state->focused) {
        if (mouseHover) {
            // Activate the box when hovering.
            state->selectedIndex = input->interactionIndex;
        }
        if ((event->code == KEY_RETURN && state->selectedIndex == input->interactionIndex)
            || (mouseHover && mouseClick(event))) {
            // Start writing on enter pressed or left click.
            input->isWriting = true;
            return true;
        }
    }
    return false;
}

bool isPrevEvent(NavigationDirection direction, Event* event) {
    int prevKeyLetter = direction == ND_HORIZONTAL ? KEY_Q : KEY_Z;
    int prevKeyArrow = direction == ND_HORIZONTAL ? KEY_LEFT : KEY_UP;

    return event->code == prevKeyLetter || event->code == prevKeyArrow;
}

bool isNextEvent(NavigationDirection direction, Event* event) {
    int nextKeyLetter = direction == ND_HORIZONTAL ? KEY_D : KEY_S;
    int nextKeyArrow = direction == ND_HORIZONTAL ? KEY_RIGHT : KEY_DOWN;

    return event->code == nextKeyLetter || event->code == nextKeyArrow;
}

void uiKeyboardNav(UIState* state, Event* event, UINavBlock blocks[], int nBlocks) {
    int selected = state->selectedIndex;
    UINavBlock* prevBlock = NULL, * curBlock = NULL, * nextBlock = NULL;
    for (int i = 0; i < nBlocks; ++i) {
        if (blocks[i].startIndex <= selected && blocks[i].endIndex >= selected) {
            prevBlock = i == 0 ? NULL : &blocks[i - 1];
            curBlock = &blocks[i];
            nextBlock = i == nBlocks - 1 ? NULL : &blocks[i + 1];
            break;
        }
    }
    if (curBlock == NULL) {
        RAGE_QUIT(10000, "UI Block not found.");
    }

    if (isPrevEvent(curBlock->direction, event) && selected > curBlock->startIndex) {
        state->selectedIndex--;
    } else if (isNextEvent(curBlock->direction, event) && selected < curBlock->endIndex) {
        state->selectedIndex++;
    } else if (prevBlock != NULL && isPrevEvent(ND_VERTICAL, event)) {
        // Go to the previous block
        state->selectedIndex = prevBlock->endIndex;
    } else if (nextBlock != NULL && isNextEvent(ND_VERTICAL, event)) {
        // Go to the next block
        state->selectedIndex = nextBlock->startIndex;
    }
}
