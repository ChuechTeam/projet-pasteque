#include "ui.h"
#include "colors.h"
#include "game_state.h"
#include <string.h>

bool mouseIntersect(Panel* panel, Event* event, int x, int y, int width, int height) {
    if (panel == NULL) {
        RAGE_QUIT(10001, "Panel is NULL.");
    }

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

ColorId uiGetToggleOptionColor(const UIState* state, const ToggleOption* option) {
    if (state->focused && state->selectedIndex == option->interactionIndex) {
        return option->toggled ? option->style.toggledActiveColorId : option->style.activeColorId;
    } else {
        return option->toggled ? option->style.toggledInactiveColorId : option->style.inactiveColorId;
    }
}


void uiDrawToggleOption(Panel* panel, UIState* state, ToggleOption* option, int x, int y, int width,
                        char* text, int interactionIndex, ToggleOptionStyle style) {
    if (panel == NULL) {
        RAGE_QUIT(10001, "Panel is NULL.");
    } else if (state == NULL) {
        RAGE_QUIT(10002, "UIState is NULL.");
    } else if (option == NULL) {
        RAGE_QUIT(10003, "ToggleOption is NULL.");
    }

    if (!option->initialized) {
        option->x = x;
        option->y = y;
        option->width = width;
        option->interactionIndex = interactionIndex;
        option->style = style;
        option->panel = panel;
        option->initialized = true;
    }

    ColorId color = uiGetToggleOptionColor(state, option);

    panelDrawLine(panel, option->x, option->y, option->width, ' ', color);
    int length = (int) strlen(text);
    int textX = (option->width - length) / 2;
    if (textX < 0) { textX = 0; }

    panelDrawText(panel, option->x + textX, option->y, text, color);
}

bool uiHandleToggleOptionEvent(UIState* state, ToggleOption* option, Event* event) {
    if (!state->focused || !option->initialized) {
        return false;
    }
    if (option->panel == NULL) {
        RAGE_QUIT(10004, "ToggleOption panel is NULL despite being initialized.");
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
    if (!input->initialized) {
        return false;
    }
    if (input->panel == NULL) {
        RAGE_QUIT(10004, "TextInput panel is NULL despite being initialized.");
    }

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

void drawNotificationPanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    NotificationPanelData* data = panelData;

    for (int i = 0; i < panel->height; ++i) {
        panelDrawLine(panel, 0, i, panel->width, ' ', data->color);
    }
    panelDrawText(panel, 1, 1, data->textWrapped, data->color);
}

Panel* uiAddNotificationPanel(struct PastequeGameState_S* gameState, int maxWidth, NotificationPanelData* data) {
    if (gameState == NULL) {
        RAGE_QUIT(10020, "GameState is NULL.");
    } else if (data == NULL) {
        RAGE_QUIT(10021, "NotificationPanelData is NULL.");
    } else if (maxWidth <= 2) {
        RAGE_QUIT(10022, "maxWidth is too low (<= 2)");
    }

    data->durationMicros = 0;
    data->color = 1;
    data->maxWidth = maxWidth;
    data->text[0] = '\0';
    data->textWrapped[0] = '\0';
    data->panel = gsAddPanel(gameState, 0, 0, maxWidth, 4, noneAdornment, &drawNotificationPanel, data);
    data->panel->visible = false;
    gsMovePanelLayer(gameState, data->panel, 10);
    // Panel will be automatically resized anyway.

    return data->panel;
}

void uiDisplayNotification(NotificationPanelData* data, char* text, ColorId color, unsigned long duration) {
    if (data == NULL) {
        RAGE_QUIT(10021, "NotificationPanelData is NULL.");
    } else if (text == NULL) {
        RAGE_QUIT(10030, "text is NULL");
    }

    // Copy the text string
    strncpy(data->text, text, NOTIF_TEXT_MAX - 1);
    data->text[NOTIF_TEXT_MAX - 1] = '\0';
    data->textDirty = true;

    data->color = color;
    data->durationMicros = duration;
    data->panel->visible = true;
}

void uiUpdateNotificationPanel(NotificationPanelData* data, unsigned long deltaMicros) {
    if (data == NULL) {
        RAGE_QUIT(10021, "NotificationPanelData is NULL.");
    }

    // Wrap the text and recalculate the location of the panel
    // when the text changes.
    if (data->textDirty) {
        panelWrapText(data->text, data->maxWidth - 2, false, data->textWrapped, NOTIF_TEXT_MAX);

        data->textWrappedLines = 1;
        int i = 0;
        while (data->textWrapped[i] != '\0') {
            if (data->textWrapped[i] == '\n') {
                data->textWrappedLines++;
            }
            i++;
        }

        data->panel->height = data->textWrappedLines + 2;
        data->panel->width = data->maxWidth;

        // Place in the lower right corner.
        data->panel->x = data->panel->pScreen->width - data->maxWidth - 1;
        data->panel->y = data->panel->pScreen->height - data->panel->height - 1;

        data->textDirty = false;
    }

    if (data->durationMicros > 0) {
        data->durationMicros -= deltaMicros;
        if (data->durationMicros <= 0) {
            data->durationMicros = 0;
            data->panel->visible = false;
        }
    }
}
