//
// Created by jeuxj on 01/05/2023.
//

#include <malloc.h>
#include "panel.h"
#include "game_state.h"
#include <string.h>
#include <ctype.h>
#include "libGameRGR2.h"

const PanelAdornment noneAdornment = {PAS_NONE, 0, -1, -1, -1};
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
    panel.layer = 0;
    panel.width = width;
    panel.height = height;
    panel.visible = true;
    panel.adornment = adornment;
    panel.drawFunc = drawFunc;
    panel.pPanelData = pPanelData;
    panel.pScreen = pScreen;
    return panel;
}

bool isEmptyPanel(const Panel* pPanel) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    }
    return pPanel->drawFunc == NULL;
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
        horizontalB = bottomLeftCorner = bottomRightCorner = "▔";
        verticalL = "▏";
        verticalR = "▕";
    } else {
        return; // Goodbye!
    }
    int minX = pPanel->x - 1;
    int minY = pPanel->y - 1;
    int maxX = pPanel->x + pPanel->width;
    int maxY = pPanel->y + pPanel->height;

    // Draw the corners first.
    drawText(pPanel->pScreen, minX, minY, topLeftCorner, adornment.colorPair);
    drawText(pPanel->pScreen, maxX, minY, topRightCorner, adornment.colorPair);
    drawText(pPanel->pScreen, minX, maxY, bottomLeftCorner, adornment.colorPair);
    drawText(pPanel->pScreen, maxX, maxY, bottomRightCorner, adornment.colorPair);

    // Draw the horizontal lines, between the corners
    for (int offsetX = 0; offsetX < pPanel->width; ++offsetX) {
        // Top horizontal line
        int charX = pPanel->x + offsetX;
        drawText(pPanel->pScreen, charX, minY, horizontalT, adornment.colorPair);

        // Bottom horizontal line
        int charY = pPanel->y + pPanel->height;
        drawText(pPanel->pScreen, charX, charY, horizontalB, adornment.colorPair);
    }

    // Draw the vertical lines, between the corners
    for (int offsetY = 0; offsetY < pPanel->height; ++offsetY) {
        // Apply any color override for vertical lines
        int color = adornment.colorPair;
        if (adornment.colorPairOverrideV != -1
            && adornment.colorPairOverrideStartY <= offsetY
            && adornment.colorPairOverrideEndY >= offsetY) {
            color = adornment.colorPairOverrideV;
        }

        // Left vertical line
        int charY = pPanel->y + offsetY;
        drawText(pPanel->pScreen, minX, charY, verticalL, color);

        // Right vertical line
        int charX = pPanel->x + pPanel->width;
        drawText(pPanel->pScreen, charX, charY, verticalR, color);
    }
}

void drawPanel(Panel* pPanel, PastequeGameState* pGameState) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    }
    if (!isEmptyPanel(pPanel) && pPanel->visible) {
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
    } else if (pText == NULL) {
        RAGE_QUIT(212, "Cannot draw a NULL string.");
    }

    // TODO: Do something when the text goes off the bounds.
    //       Wrap the text? Truncate?
    if (!isYWithinPanel(pPanel, y)) {
        // No pixels to draw.
        return;
    }

    // We do seemingly complicated stuff to get multi-line text working.
    // Actually, it's just a matter of splitting the string into lines.
    bool stringEnd = false;
    int lineBegin = 0;
    for (int i = 0; !stringEnd; ++i) {
        stringEnd = pText[i] == '\0';
        if (stringEnd || pText[i] == '\n') {
            // End of line!
            int lineLen = i - lineBegin;
            if (lineLen >= 256) {
                lineLen = 255; // Limit characters to 255 (RGR limitation)
            }

            // Copy to a buffer so we get only the line printed out (not the entire text).
            char lineBoundedStr[256];
            memcpy(lineBoundedStr, pText + lineBegin, sizeof(char) * lineLen);
            lineBoundedStr[lineLen] = '\0'; // Add the null character

            // Draw to the screen, and clamp the line before.
            char* line = lineBoundedStr;
            int localX = pPanel->x + x;
            int localY = pPanel->y + y;
            if (localX < 0) { // Negative X, we need to correct some issues related to string rendering.
                if (-localX >= lineLen) {
                    // In that case, no text will be rendered at all!
                    return;
                }
                line -= localX; // Skip x characters. Might cause issues with UTF-8.
                localX = 0;
            }
            drawText(pPanel->pScreen, localX, localY, line, clrId);

            // Setup the next line (make sure to skip the \n)
            lineBegin = i + 1;
            y++;
        }
    }
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

PanelAdornment makeAdornment(PanelAdornmentStyle style, int color) {
    PanelAdornment adornment;
    adornment.style = style;
    adornment.colorPair = color;
    adornment.colorPairOverrideV = -1;
    adornment.colorPairOverrideStartY = -1;
    adornment.colorPairOverrideEndY = -1;

    return adornment;
}

void panelDrawTextCentered(Panel* pPanel, int x, int y, char* pText, int clrId) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    } else if (isEmptyPanel(pPanel)) {
        RAGE_QUIT(211, "Cannot draw on an empty Panel.");
    } else if (pText == NULL) {
        RAGE_QUIT(212, "Cannot draw a NULL string.");
    }

    int length = (int) strlen(pText);
    if (x == -1) {
        if (length <= pPanel->width) {
            x = (pPanel->width - length) / 2;
        } else {
            x = 0;
        }
    }
    if (y == -1) {
        // Assumes single line.
        y = pPanel->height / 2;
    }

    panelDrawText(pPanel, x, y, pText, clrId);
}

void panelCenterScreen(Panel* pPanel, bool centerX, bool centerY) {
    if (pPanel == NULL) {
        RAGE_QUIT(210, "Panel is NULL.");
    }

    int x = pPanel->x;
    int y = pPanel->y;
    if (centerX) {
        x = (pPanel->pScreen->width - pPanel->width) / 2;
        if (x < 0) {
            x = 0;
        }
    }
    if (centerY) {
        y = (pPanel->pScreen->height - pPanel->height) / 2;
        if (y < 0) {
            y = 0;
        }
    }

    panelTranslate(pPanel, x, y);
}

void panelWrapText(const char* pText, int width, bool hasStoryMarkups, char outWrappedText[], int wrappedTextSize) {
    // A basic line-wrapping algorithm.

    bool stringEnd = false;
    int wordStart = 0;
    int outCursor = 0;
    int lineLength = 0; // NOTE: This also counts trailing spaces. This could be improved in the future.
    for (int i = 0; !stringEnd; ++i) {
        if (pText[i] == '\0') {
            stringEnd = true;
        }

        if (isspace(pText[i]) || stringEnd) {
            int wordEnd = i; // Exclusive
            int chars = wordEnd - wordStart;

            if (chars == 1 && (i == 0 || isspace(pText[i - 1]))) {
                // Just printing out a single space, followed by another space before
                // Like HELLO    WORLD
                if (lineLength == width) {
                    // Ignore excess spaces
                } else {
                    outWrappedText[outCursor++] = pText[i];
                    lineLength++;
                }
            } else {
                // When the word is too long for the line, add a line break.
                if ((lineLength + chars) > width) {
                    // Remove any extra spaces before the word, move the cursor back
                    // to overwrite the previous spaces.
                    while (outCursor > 0 && isspace(outWrappedText[outCursor - 1])) {
                        outCursor--;
                    }
                    outWrappedText[outCursor++] = '\n';
                    lineLength = 0;
                }

                // Copy the word to the out buffer.
                if ((outCursor + chars) > wrappedTextSize - 1) {
                    // Truncate the string when we're going out of the max size.
                    // ExcessChars = (cur + ch) - (wts - 1)
                    //             = (cur + ch) - wts + 1
                    // CharsToCopy = ch - ExcessChars
                    //             = ch - cur - ch + wts - 1
                    //             = wts - cur - 1

                    memcpy(outWrappedText + outCursor, pText + wordStart, wrappedTextSize - outCursor - 1);
                    outCursor = wrappedTextSize - 1;
                } else {
                    memcpy(outWrappedText + outCursor, pText + wordStart, chars);
                    outCursor += chars;
                }

                // Add the space after the word.
                if (outCursor < wrappedTextSize - 1 && !stringEnd) {
                    outWrappedText[outCursor++] = pText[i];
                }

                // Make sure to reset the line length when a manual line break is inserted.
                if (pText[i] == '\n') {
                    lineLength = 0;
                } else {
                    lineLength += chars + 1; // Add the space in the count too
                }
            }
            wordStart = i + 1;

            if (outCursor == wrappedTextSize - 1) {
                stringEnd = true;
            }
        } else if (hasStoryMarkups && pText[i] == '/') {
            // Exempt markups (/C, /N, etc.) from the line calculation.
            lineLength -= 2;
        }
    }

    outWrappedText[outCursor] = '\0';
}
