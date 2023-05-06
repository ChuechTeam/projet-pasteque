//
// Created by jeuxj on 01/05/2023.
//

#include <malloc.h>
#include "panel.h"
#include "game_state.h"
#include <stdbool.h>
#include <string.h>
#include "libGameRGR2.h"
#include <stdlib.h>

const PanelAdornment noneAdornment = {PAS_NONE, 0};
const Panel emptyPanel = {0};

// HELPER FUNCTIONS

bool isWithinScreen(Screen* screen, int x, int y) {
    return x >= 0 && y >= 0 && x < screen->width && y < screen->height;
}

// Local coordinates!
bool isYWithinPanel(Panel* panel, int y) {
    return y >= 0 && y < panel->height && (panel->y + y) < panel->pScreen->height;
}

void lineIntersect(Panel* panel, int start, int length, int* outStart, int* outLength) {
    if (start < 0) {
        // We're in negative X. Adjust to get X = 0.
        length += start; // Remove the "lost" length
        start = 0;
    }

    if (length <= 0) {
        // Even with that adjustment, we're totally out of bounds here.
        *outStart = 0;
        *outLength = 0;
        return;
    }

    // Find the max X coordinate (exclusive!)
    int bound = panel->x + panel->width;
    if (bound > panel->pScreen->width) {
        // The panel is going out of bounds. Limit ourselves to the screen bound.
        bound = panel->pScreen->width;
    }
    if (bound < 0) {
        // Totally out of bounds: panel is fully on negative X.
        *outStart = start;
        *outLength = 0;
    } else if (start + length >= bound) {
        // Trim off the excess length.
        *outStart = start;
        *outLength = (start + length) - bound + 1;
        if (*outLength < 0) {
            // We might get a negative value, avoid that.
            *outLength = 0;
        }
    } else {
        // All good!
        *outStart = start;
        *outLength = length;
    }
}

Panel constructPanel(int index, int x, int y, int width, int height,
                     PanelAdornment adornment, DrawPanelFunction drawFunc, void* pPanelData, Screen* pScreen) {
    if (drawFunc == NULL) {
        RAGE_QUIT(202, "Panel draw function is null.");
    }
    if (pScreen == NULL) {
        RAGE_QUIT(203, "Panel screen is null.");
    }
    if (width <= 0 || height <= 0) {
        RAGE_QUIT(204, "Panel dimensions are invalid. (width, height) = (%d, %d)", width, height);
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
    panel.freePanelDataOnDestroy = false;
    panel.pScreen = pScreen;
    return panel;
}

bool isEmptyPanel(const Panel* pPanel) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    }
    return pPanel->drawFunc == NULL;
}

void freePanelData(Panel* pPanel) {
    if (pPanel) {
        if (pPanel->freePanelDataOnDestroy) {
            free(pPanel->pPanelData);
        }
        pPanel->pPanelData = NULL;
    }
}

// An internal function for drawing the adornment
void adornPanel(Panel* pPanel) {
    const PanelAdornment adornment = pPanel->adornment;
    char* horizontalB, * horizontalT, * verticalL, * verticalR,
            * topLeftCorner, * topRightCorner, * bottomLeftCorner, * bottomRightCorner;
    if (adornment.style == PAS_SINGLE_BORDER) {
        horizontalB = horizontalT = "─";
        verticalL = verticalR = "│";
        topLeftCorner = "┌";
        topRightCorner = "┐";
        bottomLeftCorner = "└";
        bottomRightCorner = "┘";
    } else if (adornment.style == PAS_DOUBLE_BORDER) {
        horizontalT = horizontalB = "═";
        verticalL = verticalR = "║";
        topLeftCorner = "╔";
        topRightCorner = "╗";
        bottomLeftCorner = "╚";
        bottomRightCorner = "╝";
    } else if (adornment.style == PAS_CLOSE_BORDER) {
        horizontalT = topLeftCorner = topRightCorner = "▁";
        horizontalB = bottomLeftCorner = bottomRightCorner = "▁";
        verticalL = "▕";
        verticalR = "▏";
    } else {
        return; // Goodbye!
    }
    int minX = pPanel->x - 1;
    int minY = pPanel->y - 1;
    int maxX = pPanel->x + pPanel->width;
    int maxY = pPanel->y + pPanel->height;

    // Draw the corners first.
    drawText(pPanel->pScreen, minX, minY, topLeftCorner, adornment.colorPair);
    bool hasTopLeft = isWithinScreen(pPanel->pScreen, minX, minY);
    drawText(pPanel->pScreen, maxX, minY, topRightCorner, adornment.colorPair);
    bool hasTopRight = isWithinScreen(pPanel->pScreen, maxX, minY);
    drawText(pPanel->pScreen, minX, maxY, bottomLeftCorner, adornment.colorPair);
    bool hasBottomLeft = isWithinScreen(pPanel->pScreen, minX, maxY);
    drawText(pPanel->pScreen, maxX, maxY, bottomRightCorner, adornment.colorPair);
    bool hasBottomRight = isWithinScreen(pPanel->pScreen, maxX, maxY);

    // Draw the horizontal lines, between the corners
    for (int offsetX = 0; offsetX < pPanel->width; ++offsetX) {
        int charX = pPanel->x + offsetX;
        // Top horizontal line exists
        if (hasTopLeft || hasTopRight) {
            drawText(pPanel->pScreen, charX, minY, horizontalT, adornment.colorPair);
        }

        // Bottom horizontal line exists
        if (hasBottomLeft || hasBottomRight) {
            int charY = pPanel->y + pPanel->height;
            drawText(pPanel->pScreen, charX, charY, horizontalB, adornment.colorPair);
        }
    }

    // Draw the vertical lines, between the corners
    for (int offsetY = 0; offsetY < pPanel->height; ++offsetY) {
        int charY = pPanel->y + offsetY;
        // Left vertical line exists
        if (hasTopLeft || hasBottomLeft) {
            drawText(pPanel->pScreen, minX, charY, verticalL, adornment.colorPair);
        }

        // Right vertical line exists
        if (hasBottomLeft || hasBottomRight) {
            int charX = pPanel->x + pPanel->width;
            drawText(pPanel->pScreen, charX, charY, verticalR, adornment.colorPair);
        }
    }
}

void drawPanel(Panel* pPanel, PastequeGameState* pGameState) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    }
    if (!isEmptyPanel(pPanel)) {
        // Call the draw function so drawPanelX functions get called.
        pPanel->drawFunc(pPanel, pGameState, pPanel->pPanelData);
        // Then, adorn the panel, as the adornment can change inside the draw function.
        adornPanel(pPanel);
    }
}

void panelDrawLine(Panel* pPanel, int x, int y, int w, char ch, int clrId) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    } else if (isEmptyPanel(pPanel)) {
        RAGE_QUIT(211, "Cannot draw on an empty Panel.");
    }

    lineIntersect(pPanel, x, w, &x, &w);
    if (w == 0 || !isYWithinPanel(pPanel, y)) {
        // No pixels to draw.
        return;
    }

    int localX = pPanel->x + x;
    int localY = pPanel->y + y;

    drawLine(pPanel->pScreen, localX, localY, w, ch, clrId);
}

void panelDrawText(Panel* pPanel, int x, int y, char* pText, int clrId) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    } else if (isEmptyPanel(pPanel)) {
        RAGE_QUIT(211, "Cannot draw on an empty Panel.");
    }

    // TODO: Do something when the text goes off the bounds.
    //       Wrap the text? Truncate?
    if (!isYWithinPanel(pPanel, y)) {
        // No pixels to draw.
        return;
    }

    int localX = pPanel->x + x;
    int localY = pPanel->y + y;
    int length = (int) strlen(pText);
    if (localX < 0) { // Negative X, we need to correct some issues related to string rendering.
        if (-localX >= length) {
            // In that case, no text will be rendered at all!
            return;
        }
        pText -= localX; // Skip x characters. Might cause issues with UTF-8.
        length += localX; // Remove characters that we skipped on the length.
        localX = 0;
    }

    drawText(pPanel->pScreen, localX, localY, pText, clrId);
}

void panelTranslate(Panel* pPanel, int x, int y) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    } else if (isEmptyPanel(pPanel)) {
        RAGE_QUIT(211, "Cannot translate an empty Panel.");
    }
    pPanel->x = x;
    pPanel->y = y;
}
