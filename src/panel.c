//
// Created by jeuxj on 01/05/2023.
//

#include <malloc.h>
#include "panel.h"
#include "game_state.h"
#include <stdbool.h>
#include <string.h>
#include "libGameRGR2.h"

const PanelAdornment noneAdornment = {PAS_NONE, 0};
const Panel emptyPanel = {0};

bool isWithinScreen(Screen* screen, int x, int y) {
    return x >= 0 && y >= 0 && x < screen->width && y < screen->height;
}

void validatePanelCoords(int x, int y, int width, int height, const Screen* pScreen) {
    if (x < 0 || y < 0 || width < 0 || height < 0) {
        RAGE_QUIT(200, "Invalid panel rectangle: \n\t"
                       "RECT: (x, y, width, height) = (%d, %d, %d, %d)", x, y, width, height);
    }
    if ((x + width) >= pScreen->width || (y + height) >= pScreen->height) {
        RAGE_QUIT(201, "Panel rectangle is out of screen:\n\t"
                       "RECT: (x, y, width, height) = (%d, %d, %d, %d)\n\t"
                       "SCREEN: (width, height) = (%d, %d)", x, y, width, height, pScreen->width, pScreen->height);
    }
}

Panel constructPanel(int index, int x, int y, int width, int height,
                     PanelAdornment adornment, DrawPanelFunction drawFunc, void* pPanelData, Screen* pScreen) {
    // Validate the parameters
    validatePanelCoords(x, y, width, height, pScreen);
    if (drawFunc == NULL) {
        RAGE_QUIT(202, "Panel draw function is null.");
    }
    if (pScreen == NULL) {
        RAGE_QUIT(203, "Panel screen is null.");
    }
    Panel panel;
    panel.index = index;
    panel.x = x;
    panel.y = y;
    panel.width = width;
    panel.height = height;
    panel.adornment = adornment;
    panel.drawFunc = drawFunc;
    panel.pPanelData = pPanelData;
    panel.pScreen = pScreen;
    return panel;
}

bool isEmptyPanel(const Panel* pPanel) {
    return pPanel->drawFunc == NULL;
}

void freePanelData(Panel* pPanel) {
    if (pPanel) {
        free(pPanel->pPanelData);
    }
}

// The GameState used while drawing a panel, used for dirty pixels.
// For convenience, this is global state because passing the GameState
// for every panelDrawX function would be tedious. Or would require another struct
// to be "concise".
PastequeGameState* panelDrawGameState = NULL;

// A helper function for above variable. Make sure to check panelDrawGameState before.
bool registerFilledPixelUnsure(int x, int y) {
    if (isWithinScreen(panelDrawGameState->screen, x, y)) {
        panelDrawGameState->curFilledPixels[y][x] = 1;
        return true;
    }
    return false;
}

// Same as registerFilledPixelUnsure, but now you're sure that the pixels exists!
void registerFilledPixel(int x, int y) {
    panelDrawGameState->curFilledPixels[y][x] = 1;
}

// An internal function for drawing the adornment
void adornPanel(Panel* pPanel) {
    const PanelAdornment adornment = pPanel->adornment;
    char* horizontal, * vertical, * topLeftCorner, * topRightCorner, * bottomLeftCorner, * bottomRightCorner;
    if (adornment.style == PAS_SINGLE_BORDER) {
        horizontal = "─";
        vertical = "│";
        topLeftCorner = "┌";
        topRightCorner = "┐";
        bottomLeftCorner = "└";
        bottomRightCorner = "┘";
    } else if (adornment.style == PAS_DOUBLE_BORDER) {
        horizontal = "═";
        vertical = "║";
        topLeftCorner = "╔";
        topRightCorner = "╗";
        bottomLeftCorner = "╚";
        bottomRightCorner = "╝";
    } else {
        return; // Goodbye!
    }
    int minX = pPanel->x - 1;
    int minY = pPanel->y - 1;
    int maxX = pPanel->x + pPanel->width;
    int maxY = pPanel->y + pPanel->height;

    drawText(pPanel->pScreen, minX, minY, topLeftCorner, adornment.colorPair);
    bool hasTopLeft = registerFilledPixelUnsure(minX, minY);
    drawText(pPanel->pScreen, maxX, minY, topRightCorner, adornment.colorPair);
    bool hasTopRight = registerFilledPixelUnsure(maxX, minY);
    drawText(pPanel->pScreen, minX, maxY, bottomLeftCorner, adornment.colorPair);
    bool hasBottomLeft = registerFilledPixelUnsure(minX, maxY);
    drawText(pPanel->pScreen, maxX, maxY, bottomRightCorner, adornment.colorPair);
    bool hasBottomRight = registerFilledPixelUnsure(maxX, maxY);

    for (int offsetX = 0; offsetX < pPanel->width; ++offsetX) {
        int charX = pPanel->x + offsetX;
        // Top horizontal line exists
        if (hasTopLeft || hasTopRight) {
            drawText(pPanel->pScreen, charX, minY, horizontal, adornment.colorPair);
            registerFilledPixelUnsure(charX, minY);
        }

        // Bottom horizontal line exists
        if (hasBottomLeft || hasBottomRight) {
            int charY = pPanel->y + pPanel->height;
            drawText(pPanel->pScreen, charX, charY, horizontal, adornment.colorPair);
            registerFilledPixelUnsure(charX, charY);
        }
    }

    for (int offsetY = 0; offsetY < pPanel->height; ++offsetY) {
        int charY = pPanel->y + offsetY;
        // Left vertical line exists
        if (hasTopLeft || hasBottomLeft) {
            drawText(pPanel->pScreen, minX, charY, vertical, adornment.colorPair);
            registerFilledPixelUnsure(minX, charY);
        }

        // Right vertical line exists
        if (hasBottomLeft || hasBottomRight) {
            int charX = pPanel->x + pPanel->width;
            drawText(pPanel->pScreen, charX, charY, vertical, adornment.colorPair);
            registerFilledPixelUnsure(charX, charY);
        }
    }
}

void drawPanel(Panel* pPanel, PastequeGameState* pGameState) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    }
    if (!isEmptyPanel(pPanel)) {
        panelDrawGameState = pGameState;
        pPanel->drawFunc(pPanel, pGameState, pPanel->pScreen);
        adornPanel(pPanel);
        panelDrawGameState = NULL;
    }
}

void panelDrawLine(Panel* pPanel, int x, int y, int w, char ch, int clrId) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    } else if (isEmptyPanel(pPanel)) {
        RAGE_QUIT(211, "Cannot draw on an empty Panel.");
    }
    // Remove any out of bound pixels.
    int outOfBoundsPixels = (w + x) - pPanel->width + 1;
    if (outOfBoundsPixels > 0) {
        w -= outOfBoundsPixels;
    }
    int localX = pPanel->x + x;
    int localY = pPanel->y + y;
    if (localY >= pPanel->y + pPanel->height || w == 0) {
        // No pixels to draw.
        return;
    }

    drawLine(pPanel->pScreen, localX, localY, w, ch, clrId);

    // Register any filled pixels for dirty pixels detection.
    if (panelDrawGameState) {
        int finalX = localX + w; // Exclusive max
        for (int curX = localX; curX < finalX; ++curX) {
            panelDrawGameState->curFilledPixels[localY][curX] = 1;
        }
    }
}

void panelDrawText(Panel* pPanel, int x, int y, char* pText, int clrId) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    } else if (isEmptyPanel(pPanel)) {
        RAGE_QUIT(211, "Cannot draw on an empty Panel.");
    }
    // TODO: Do something when the text goes off the bounds.
    //       Wrap the text? Truncate?
    int localX = pPanel->x + x;
    int localY = pPanel->y + y;
    if (localY >= pPanel->y + pPanel->height) {
        // No pixels to draw.
        return;
    }
    drawText(pPanel->pScreen, localX, localY, pText, clrId);

    // Register any filled pixels for dirty pixels detection.
    if (panelDrawGameState) {
        int finalX = localX + (int) strlen(pText); // Exclusive max
        int maxX = pPanel->x + pPanel->width; // Exclusive too
        if (finalX > maxX) {
            finalX = maxX;
        }
        for (int curX = localX; curX < finalX; ++curX) {
            panelDrawGameState->curFilledPixels[localY][curX] = 1;
        }
    }
}

void panelTranslate(Panel* pPanel, int x, int y) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    } else if (isEmptyPanel(pPanel)) {
        RAGE_QUIT(211, "Cannot translate an empty Panel.");
    }
    validatePanelCoords(x, y, pPanel->width, pPanel->height, pPanel->pScreen);
    pPanel->x = x;
    pPanel->y = y;
}
