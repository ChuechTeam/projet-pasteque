/**
 * colors.h
 * --------
 * Provides cool and vibrant colors for the game. Notably: black and white.
 */
#ifndef PROJET_PASTEQUE_COLORS_H
#define PROJET_PASTEQUE_COLORS_H

#include "libGameRGR2.h"

typedef int ColorId;

#define PASTEQUE_COLOR_WHITE 1
#define PASTEQUE_COLOR_BLACK 2
#define PASTEQUE_COLOR_BLANK 8

/**
 * Initializes all the color and color pairs. Must be called in init.
 */
void initColors();

#endif //PROJET_PASTEQUE_COLORS_H
