#ifndef PROJET_PASTEQUE_COLORS_H
#define PROJET_PASTEQUE_COLORS_H

#include "libGameRGR2.h"

#define PASTEQUE_COLOR_WHITE 1
#define PASTEQUE_COLOR_BLANK 8

inline void initColors() {
    setColorPair(1, COLOR_WHITE, COLOR_BLACK);
    setColorPair(8, COLOR_BLACK, COLOR_BLACK);
}

#endif //PROJET_PASTEQUE_COLORS_H
