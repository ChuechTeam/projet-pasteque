#include "colors.h"

void initTextColor(int id, int color) {
    setColorPair(id, color, COLOR_BLACK);
}

void initBGColorWhite(int id, int color) {
    setColorPair(id, COLOR_WHITE, color);
}

void initBGColorBlack(int id, int color) {
    setColorPair(id, COLOR_BLACK, color);
}

int getTextColor(short id) {
    short useless, textColor;
    pair_content(id, &textColor, &useless);
    return textColor;
}

void initColors() {
    // See https://www.ditig.com/256-colors-cheat-sheet
    setColor(COLOR_WHITE, 255, 255, 255);
    setColor(COLOR_BLACK, 0, 0, 0);
    setColorPair(PASTEQUE_COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
    setColorPair(PASTEQUE_COLOR_BLACK, COLOR_BLACK, COLOR_WHITE);

    // Replace yellow with a more vivid color for terminals that support it.
    setColor(184, 252, 255, 30);
    initTextColor(PASTEQUE_COLOR_YELLOW, 184);

    setColorPair(PASTEQUE_COLOR_BLANK, COLOR_BLACK, COLOR_BLACK);

    initTextColor(PASTEQUE_COLOR_BLUE, 33);

    initTextColor(PASTEQUE_COLOR_RED, 160);

    initTextColor(PASTEQUE_COLOR_ORANGE, 208);

    initTextColor(PASTEQUE_COLOR_GREEN, 155);

    initTextColor(PASTEQUE_COLOR_TOMATO, 202);

    initTextColor(PASTEQUE_COLOR_FUSCHIA, 165);

    initTextColor(PASTEQUE_COLOR_TURQUOISE, 45);

    setColorPair(PASTEQUE_COLOR_YELLOW_HIGHLIGHT_BG, COLOR_BLACK, COLOR_YELLOW);

    initBGColorBlack(PASTEQUE_COLOR_YELLOW_FOCUS_BG, 202);

    initBGColorWhite(PASTEQUE_COLOR_RED_BLINK1_BG, 204);

    initBGColorBlack(PASTEQUE_COLOR_RED_BLINK2_BG, 196);

    setColorPair(PASTEQUE_COLOR_GREY_25_BG, COLOR_WHITE, 237);

    setColorPair(PASTEQUE_COLOR_BLUE_ON_WHITE, getTextColor(PASTEQUE_COLOR_BLUE), COLOR_WHITE);
}