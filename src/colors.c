#include "colors.h"

void initTextColor(int id, int r, int g, int b) {
    setColor(id, r, g, b);
    setColorPair(id, id, COLOR_BLACK);
}

void initBGColorWhite(int id, int r, int g, int b) {
    setColor(id, r, g, b);
    setColorPair(id, COLOR_WHITE, id);
}

void initBGColorBlack(int id, int r, int g, int b) {
    setColor(id, r, g, b);
    setColorPair(id, COLOR_BLACK, id);
}

void initColors() {
    setColor(COLOR_WHITE, 255, 255, 255);
    setColor(COLOR_BLACK, 0, 0, 0);
    setColorPair(PASTEQUE_COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
    setColorPair(PASTEQUE_COLOR_BLACK, COLOR_BLACK, COLOR_WHITE);

    initTextColor(PASTEQUE_COLOR_YELLOW, 255, 241, 80);

    setColorPair(PASTEQUE_COLOR_BLANK, COLOR_BLACK, COLOR_BLACK);

    initTextColor(PASTEQUE_COLOR_BLUE, 0, 114, 255);

    initTextColor(PASTEQUE_COLOR_RED, 255, 51, 36);

    initTextColor(PASTEQUE_COLOR_ORANGE, 253, 158, 22);

    initTextColor(PASTEQUE_COLOR_GREEN, 34, 255, 60);

    initTextColor(PASTEQUE_COLOR_TOMATO, 255, 112, 49);

    initTextColor(PASTEQUE_COLOR_FUSCHIA, 255, 49, 238);

    initTextColor(PASTEQUE_COLOR_TURQUOISE, 47, 255, 218);

    setColorPair(PASTEQUE_COLOR_YELLOW_HIGHLIGHT_BG, COLOR_BLACK, COLOR_YELLOW);

    initBGColorBlack(PASTEQUE_COLOR_YELLOW_FOCUS_BG, 213, 147, 20);

    initBGColorWhite(PASTEQUE_COLOR_RED_BLINK1_BG, 255, 65, 65);

    initBGColorBlack(PASTEQUE_COLOR_RED_BLINK2_BG, 255, 134, 134);

    setColorPair(PASTEQUE_COLOR_BLUE_ON_WHITE, PASTEQUE_COLOR_BLUE, COLOR_WHITE);
}