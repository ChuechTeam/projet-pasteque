#include "ui.h"
#include "colors.h"
#include <string.h>

void drawToggleOption(Panel* panel, const ToggleOption* option, UIState* state) {
    ColorId color;
    if (option->toggled) {
        color = option->style.toggledColorId;
    } else if (state->selectedIndex == option->interactionIndex) {
        color = option->style.activeColorId;
    } else {
        color = option->style.inactiveColorId;
    }

    panelDrawLine(panel, option->x, option->y, option->width, ' ', color);
    int length = (int) strlen(option->text);
    int textX = (option->width - length) / 2;
    if (textX < 0) { textX = 0; }

    panelDrawText(panel, option->x + textX, option->y, option->text, color);
}

ToggleOption
makeToggleOption(Panel* panel, int x, int y, int width, char* text, int interactionIndex, ToggleOptionStyle style) {
    ToggleOption opt = {panel, x, y, width, text, interactionIndex, style, false};
    return opt;
}

bool handleToggleOptionEvent(UIState* state, ToggleOption* option, Event* event) {
    if (event->code == KEY_MOUSE) {
        int minX = option->x + option->panel->x;
        int maxX = minX + option->width;
        int minY = option->y + option->panel->y;
        int maxY = minY + 1;

        if (event->mouseEvent.x >= minX && event->mouseEvent.x < maxX
            && event->mouseEvent.y >= minY && event->mouseEvent.y < maxY) {
            state->selectedIndex = option->interactionIndex;
        }
    }
    return false;
}
