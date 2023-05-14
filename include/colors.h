/**
 * colors.h
 * --------
 * Provides cool and vibrant colors for the game. Notably: black and white.
 */
#ifndef PROJET_PASTEQUE_COLORS_H
#define PROJET_PASTEQUE_COLORS_H

#include "libGameRGR2.h"

typedef int ColorId;

// Primary colors (default in terminal 8-color)
#define PASTEQUE_COLOR_YELLOW COLOR_YELLOW
#define PASTEQUE_COLOR_BLUE COLOR_BLUE

// Extended colors
#define PASTEQUE_EXT_COLOR(n) (16+n)
#define PASTEQUE_COLOR_TOMATO PASTEQUE_EXT_COLOR(1)
#define PASTEQUE_COLOR_FUSCHIA PASTEQUE_EXT_COLOR(2)
#define PASTEQUE_COLOR_TURQUOISE PASTEQUE_EXT_COLOR(3)
#define PASTEQUE_COLOR_GREEN PASTEQUE_EXT_COLOR(4)
#define PASTEQUE_COLOR_ORANGE PASTEQUE_EXT_COLOR(5)

// Those colors are bugged for some reason when using COLOR_X macros.
#define PASTEQUE_COLOR_RED PASTEQUE_EXT_COLOR(15)
#define PASTEQUE_COLOR_WHITE PASTEQUE_EXT_COLOR(16)
#define PASTEQUE_COLOR_BLACK PASTEQUE_EXT_COLOR(17)
#define PASTEQUE_COLOR_BLANK PASTEQUE_EXT_COLOR(18)

// Supplementary background colors
#define PASTEQUE_BG_COLOR(n) (16+32+n)
#define PASTEQUE_COLOR_YELLOW_HIGHLIGHT_BG PASTEQUE_BG_COLOR(1)
#define PASTEQUE_COLOR_YELLOW_FOCUS_BG PASTEQUE_BG_COLOR(2)
#define PASTEQUE_COLOR_RED_BLINK1_BG PASTEQUE_BG_COLOR(3)
#define PASTEQUE_COLOR_RED_BLINK2_BG PASTEQUE_BG_COLOR(4)

// Supplementary specific background/foreground colors
#define PASTEQUE_COMB_COLOR(n) (16+32+16+n)
#define PASTEQUE_COLOR_BLUE_ON_WHITE PASTEQUE_COMB_COLOR(1)

/**
 * Initializes all the color and color pairs. Must be called in init.
 */
void initColors();

#endif //PROJET_PASTEQUE_COLORS_H
