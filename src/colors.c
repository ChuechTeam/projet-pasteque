#include "colors.h"

void initColors() {
    setColorPair(1, COLOR_WHITE, COLOR_BLACK);
    setColorPair(2, COLOR_BLACK, COLOR_WHITE);
    setColor(COLOR_YELLOW, 255, 241, 80);
    setColorPair(3, COLOR_BLACK, COLOR_YELLOW);
    setColorPair(8, COLOR_BLACK, COLOR_BLACK);
}