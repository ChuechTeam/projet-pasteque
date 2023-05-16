#include "scenes/crush_scene.h"
#include <stdlib.h>
#include "colors.h"
#include <stdbool.h>
#include <string.h>
#include "board.h"
#include "scenes/main_menu_scene.h"
#include <limits.h>

#define MILLI_IN_MICROS 1000L
#define MICROS(millis) (millis*MILLI_IN_MICROS)
#define BLINKING_DURATION_MICROS MICROS(1000L)
#define GRAVITY_PERIOD_MICROS MICROS(200L)

typedef enum {
    CPS_WAITING_INPUT,
    CPS_EVALUATING_BOARD,
    CPS_BLINKING,
    CPS_GRAVITY,
    CPS_GAME_OVER
} CrushPlayState;

struct CrushData_S {
    // GAME STATE
    int score;
    CrushBoard* board;
    CrushPlayState playState;
    unsigned long animTimeMicros;

    // MESSAGES
    char message[128];
    ColorId messageColor;
    long messageDurationMicros;

    // INPUT
    Point cursor;
    // True when the cursor focuses on a cell to swap with another.
    bool cursorSwapping;
    // The cells to highlight while swapping with the cursor
    Point swapNeighbors[4];
    int numSwapNeighbors;
    CrushInputMethod inputMethod;

    // PANELS
    Panel* boardPanel;
    Panel* scorePanel;
    Panel* messagePanel;
};

CrushData* makeCrushData(BoardSizePreset sizePreset, int width, int height, char symbols, CrushInputMethod inputMethod) {
    CrushData* data = calloc(1, sizeof(CrushData));
    if (data == NULL) {
        RAGE_QUIT(2001, "Failed to allocate CrushData.");
    }

    // This will validate width, height, and the symbols.
    data->board = makeCrushBoard(sizePreset, width, height, symbols);
    data->inputMethod = inputMethod;
    data->message[0] = '\0';
    data->messageColor = PASTEQUE_COLOR_WHITE;
    return data;
}

char* cellToStr(CrushCell cell, ColorId* outColor) {
    // Can be adjusted for a different theme, etc.
    switch (cell.sym) {
        case 0:
            *outColor = PASTEQUE_COLOR_WHITE;
            return " ";
        case 1:
            *outColor = PASTEQUE_COLOR_GREEN;
            return "O";
        case 2:
            *outColor = PASTEQUE_COLOR_BLUE;
            return "&";
        case 3:
            *outColor = PASTEQUE_COLOR_TOMATO;
            return "#";
        case 4:
            *outColor = PASTEQUE_COLOR_YELLOW;
            return "$";
        case 5:
            *outColor = PASTEQUE_COLOR_FUSCHIA;
            return "@";
        case 6:
            *outColor = PASTEQUE_COLOR_TURQUOISE;
            return "*";
        default:
            *outColor = PASTEQUE_COLOR_WHITE;
            return "?";
    }
}

void drawBoardPanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    CrushData* data = panelData;
    CrushBoard* board = data->board;
    for (int c = 0; c < board->cellCount; c++) {
        CrushCell cell = board->cells[c];
        Point pos = boardCellIndexToPos(data->board, c);

        ColorId color;
        char* cellStr = cellToStr(cell, &color);

        // Override the colors in some situations
        if (cell.highlightedNeighbor) {
            color = PASTEQUE_COLOR_YELLOW_HIGHLIGHT_BG;
        } else if (cell.markedForDestruction && data->playState == CPS_BLINKING) {
            color = (gameState->gameTime % MICROS(200)) < MICROS(100) ?
                    PASTEQUE_COLOR_RED_BLINK1_BG :
                    PASTEQUE_COLOR_RED_BLINK2_BG;
        } else if (pointsEqual(data->cursor, pos)) {
            if (data->cursorSwapping) {
                color = PASTEQUE_COLOR_YELLOW_FOCUS_BG;
            } else {
                color = PASTEQUE_COLOR_YELLOW_HIGHLIGHT_BG;
            }
        }

        panelDrawText(panel, 2 * pos.x, pos.y, cellStr, color);
    }
}

void drawScorePanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    CrushData* data = panelData;

    char buffer[64];
    sprintf(buffer, "Score : %d", data->score);

    panelDrawText(panel, 0, 0, buffer, PASTEQUE_COLOR_BLUE);
}

void drawMessagePanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    CrushData* data = panelData;

    if (data->messageDurationMicros > 0) {
        panelDrawText(panel, 0, 0, data->message, data->messageColor);
    }
}

void displayMessage(CrushData* data, const char* str, ColorId color, long durationMicros) {
    strncpy(data->message, str, 128);
    data->messageColor = color;
    data->messageDurationMicros = durationMicros;
}

void moveCursorDelta(CrushBoard* board, Point* cursor, int deltaX, int deltaY) {
    cursor->x += deltaX;
    cursor->y += deltaY;

    if (cursor->x < 0) { cursor->x = 0; }
    if (cursor->x >= board->width) { cursor->x = board->width - 1; }

    if (cursor->y < 0) { cursor->y = 0; }
    if (cursor->y >= board->height) { cursor->y = board->height - 1; }
}

void toggleCursorSwapping(CrushData* data) {
    if (data->cursorSwapping) {
        data->cursorSwapping = false;
        for (int i = 0; i < data->numSwapNeighbors; ++i) {
            CELL_PT(data->board, data->swapNeighbors[i]).highlightedNeighbor = false;
        }
        data->numSwapNeighbors = 0;
    } else {
        data->cursorSwapping = true;
        boardGetNeighbors(data->board, data->cursor, data->swapNeighbors, &data->numSwapNeighbors);
        for (int i = 0; i < data->numSwapNeighbors; ++i) {
            CELL_PT(data->board, data->swapNeighbors[i]).highlightedNeighbor = true;
        }
    }
}

void runCursorMoveAction(CrushData* data, int deltaX, int deltaY) {
    if (!data->cursorSwapping) {
        moveCursorDelta(data->board, &data->cursor, deltaX, deltaY);
    } else {
        // Exit cursor swap (remove highlight bools)
        toggleCursorSwapping(data);

        // Swap cells!
        Point targetCellPos = {.x= data->cursor.x + deltaX, .y=data->cursor.y + deltaY};
        SwapResult result = boardSwapCells(data->board, data->cursor, targetCellPos, false);

        switch (result) {
            case SR_SUCCESS:
                data->playState = CPS_EVALUATING_BOARD;
                break;
            case SR_NO_MATCH:
                displayMessage(data, "Impossible d'échanger pour ne former aucune ligne.", PASTEQUE_COLOR_RED, MICROS(3000));
                break;
            case SR_EMPTY_CELLS:
                displayMessage(data, "Impossible d'échanger avec une case vide.", PASTEQUE_COLOR_RED, MICROS(3000));
                break;
            case SR_OUT_OF_BOUNDS:
                displayMessage(data, "Impossible d'échanger avec une case inexistante", PASTEQUE_COLOR_RED, MICROS(3000));
                break;
        }
    }
}

void gameOver(CrushData* data) {
    data->playState = CPS_GAME_OVER;
    displayMessage(data, "GAME OVER ! (Appuyez sur P pour revenir au menu)", PASTEQUE_COLOR_ORANGE, LONG_MAX);
}

// ----------
// GAME FUNCTIONS (Init, Event, Update, Draw, Finish)
// ----------

void crushInit(PastequeGameState* gameState, CrushData* data) {
    // Leave some space for horizontal spaces.
    PanelAdornment boardAdorn = makeAdornment(PAS_CLOSE_BORDER, PASTEQUE_COLOR_WHITE);
    data->boardPanel = gsAddPanel(gameState, 2, 2, 2 * data->board->width - 1, data->board->height,
                                  boardAdorn, &drawBoardPanel, data);

    data->scorePanel = gsAddPanel(gameState, 2, data->boardPanel->y + data->boardPanel->height + 1, 32, 1,
                                  noneAdornment, &drawScorePanel, data);

    data->messagePanel = gsAddPanel(gameState, 2, data->scorePanel->y + 2, 50, 2,
                                    noneAdornment, &drawMessagePanel, data);
}

void crushUpdate(PastequeGameState* gameState, CrushData* data, unsigned long deltaTime) {
    if (data->playState == CPS_EVALUATING_BOARD) {
        bool markedSomething = boardMarkAlignedCells(data->board, &data->score);

        if (markedSomething) {
            data->playState = CPS_BLINKING;
        } else if (boardAnySwapPossible(data->board)) {
            data->playState = CPS_WAITING_INPUT;
        } else {
            gameOver(data);
        }

    } else if (data->playState == CPS_BLINKING) {
        // Blinking color is handled in drawBoardPanel.
        data->animTimeMicros += deltaTime;
        if (data->animTimeMicros >= BLINKING_DURATION_MICROS) {
            data->animTimeMicros = 0;

            // Destroy all marked cells after blinking.
            boardDestroyMarked(data->board);
            data->playState = CPS_GRAVITY;
        }
    }
    if (data->playState == CPS_GRAVITY) {
        data->animTimeMicros += deltaTime;
        if (data->animTimeMicros >= GRAVITY_PERIOD_MICROS) {
            data->animTimeMicros = 0;

            bool gravityDidSomething = boardGravityTick(data->board);
            if (gravityDidSomething) {
                // Continue once we reach GRAVITY_PERIOD_MICROS again.
            } else {
                // Reevaluate the board if we have any chain reaction.
                data->playState = CPS_EVALUATING_BOARD;
            }
        }
    }

    // Update the message duration timer
    if (data->messageDurationMicros > 0) {
        data->messageDurationMicros -= (long)deltaTime;
        if (data->messageDurationMicros < 0) { data->messageDurationMicros = 0; }
    }
}

void crushEvent(PastequeGameState* gameState, CrushData* data, Event* pEvent) {
    // Move the cursor according to input method settings
    const bool isZQSD = (data->inputMethod & CIM_ZQSD_SPACE_CURSOR) != 0;
    const bool isArrows = (data->inputMethod & CIM_ARROWS_ENTER_CURSOR) != 0;
    const KeyCode code = pEvent->code;

    if (data->playState == CPS_WAITING_INPUT) {
        if (isZQSD && code == KEY_Z || isArrows && code == KEY_UP) {
            runCursorMoveAction(data, 0, -1);
        } else if (isZQSD && code == KEY_S || isArrows && code == KEY_DOWN) {
            runCursorMoveAction(data, 0, 1);
        } else if (isZQSD && code == KEY_Q || isArrows && code == KEY_LEFT) {
            runCursorMoveAction(data, -1, 0);
        } else if (isZQSD && code == KEY_D || isArrows && code == KEY_RIGHT) {
            runCursorMoveAction(data, 1, 0);
        } else if (isZQSD && code == KEY_SPACE || isArrows && code == KEY_RETURN) {
            toggleCursorSwapping(data);
        }
    }
    if (code == KEY_P && data->playState == CPS_GAME_OVER) {
        gsSwitchScene(gameState, SN_MAIN_MENU, makeMainMenuData());
    }

}

void crushDrawBackground(PastequeGameState* gameState, CrushData* data, Screen* pScreen) {
}

void crushDrawForeground(PastequeGameState* gameState, CrushData* data, Screen* pScreen) {

}

void crushFinish(PastequeGameState* gameState, CrushData* data) {
    if (data->board) {
        free(data->board);
    }
}
