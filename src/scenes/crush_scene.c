#include "scenes/crush_scene.h"
#include <stdlib.h>
#include "colors.h"
#include <stdbool.h>
#include "board.h"

#define MILLI_IN_MICROS 1000L
#define BLINKING_DURATION_MICROS (1000L*MILLI_IN_MICROS)
#define GRAVITY_PERIOD_MICROS (200L*MILLI_IN_MICROS)

typedef enum {
    CPS_WAITING_INPUT,
    CPS_EVALUATING_BOARD,
    CPS_BLINKING,
    CPS_GRAVITY
} CrushPlayState;

struct CrushData_S {
    int score;
    CrushBoard* board;
    Panel* boardPanel;
    Panel* scorePanel;

    CrushPlayState playState;
    unsigned long animTimeMicros;

    Point cursor;
    // True when the cursor focuses on a cell to swap with another.
    bool cursorSwapping;
    // The cells to highlight while swapping with the cursor
    Point swapNeighbors[4];
    int numSwapNeighbors;
    CrushInputMethod inputMethod;
};

CrushData* makeCrushData(int width, int height, char symbols, CrushInputMethod inputMethod) {
    CrushData* data = calloc(1, sizeof(CrushData));
    if (data == NULL) {
        RAGE_QUIT(2001, "Failed to allocate CrushData.");
    }

    // This will validate width, height, and the symbols.
    data->board = makeCrushBoard(width, height, symbols);
    data->inputMethod = inputMethod;
    return data;
}

char* cellToStr(CrushCell cell) {
    // Can be adjusted for a different theme, etc.
    switch (cell.sym) {
        case 0:
            return " ";
        case 1:
            return "O";
        case 2:
            return "&";
        case 3:
            return "#";
        case 4:
            return "$";
        case 5:
            return "@";
        case 6:
            return "%";
        default:
            return "?";
    }
}

void drawBoardPanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    CrushData* data = panelData;
    CrushBoard* board = data->board;
    for (int c = 0; c < board->cellCount; c++) {
        CrushCell cell = board->cells[c];
        Point pos = boardCellIndexToPos(data->board, c);

        ColorId color = PASTEQUE_COLOR_WHITE;
        if (cell.highlightedNeighbor) {
            color = PASTEQUE_COLOR_YELLOW_HIGHLIGHT_BG;
        } else if (cell.markedForDestruction && data->playState == CPS_BLINKING) {
            color = (gameState->gameTime % (200 * MILLI_IN_MICROS)) < (100 * MILLI_IN_MICROS) ?
                    PASTEQUE_COLOR_RED_BLINK1_BG :
                    PASTEQUE_COLOR_RED_BLINK2_BG;
        } else if (pointsEqual(data->cursor, pos)) {
            if (data->cursorSwapping) {
                color = PASTEQUE_COLOR_YELLOW_FOCUS_BG;
            } else {
                color = PASTEQUE_COLOR_YELLOW_HIGHLIGHT_BG;
            }
        }

        panelDrawText(panel, 2 * pos.x, pos.y, cellToStr(cell), color);
    }
}

void drawScorePanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    CrushData* data = panelData;

    char buffer[64];
    sprintf(buffer, "Score : %d", data->score);

    panelDrawText(panel, 0, 0, buffer, PASTEQUE_COLOR_BLUE);
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
        // TODO: do something when 0 cells swappable?
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
        bool done = boardSwapCells(data->board, data->cursor, targetCellPos);
        if (done) {
            data->playState = CPS_EVALUATING_BOARD;
        } else {
            // Some feedback maybe?
        }
    }
}

// ----------
// GAME FUNCTIONS (Init, Event, Update, Draw, Finish)
// ----------

void crushInit(PastequeGameState* gameState, CrushData* data) {
    // Leave some space for horizontal spaces.
    PanelAdornment boardAdorn = {.style = PAS_CLOSE_BORDER, .colorPair=PASTEQUE_COLOR_WHITE};
    data->boardPanel = gsAddPanel(gameState, 2, 2, 2 * data->board->width - 1, data->board->height,
                                  boardAdorn, &drawBoardPanel, data);

    data->scorePanel = gsAddPanel(gameState, 2, data->boardPanel->y + data->boardPanel->height + 1, 32, 1,
                                  noneAdornment, &drawScorePanel, data);
}

void crushUpdate(PastequeGameState* gameState, CrushData* data, unsigned long deltaTime) {
    if (data->playState == CPS_EVALUATING_BOARD) {
        bool markedSomething = boardMarkAlignedCells(data->board, &data->score);

        if (markedSomething) {
            data->playState = CPS_BLINKING;
        } else {
            data->playState = CPS_WAITING_INPUT;
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
