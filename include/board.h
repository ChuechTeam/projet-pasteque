#ifndef PROJET_PASTEQUE_BOARD_H
#define PROJET_PASTEQUE_BOARD_H

#include <stdbool.h>

#define BOARD_WIDTH_MAX 100
#define BOARD_HEIGHT_MAX 100
#define MAX_SYMBOLS 6
#define MAX_TRICKS 50
#define MAX_TRICK_LENGTH 60

// Helper macros for manipulating the cells array.

// Returns the cell at location (x, y) in the board. Fails if the coordinates are out of bounds.
#define CELL(board, x, y) board->cells[boardPosToIndexValidated(board, x, y, __FILE__, __LINE__)]
#define CELL_LP(board, x, y) board->cells[loopBackY(board, y)*board->width + loopBackX(board, x)]
#define CELL_PT(board, point) CELL(board, point.x, point.y)

typedef struct {
    // Number from 0 to 6 currently.
    // 0 = empty
    // 1 to 6 = symbols
    char sym;
    bool highlightedNeighbor;
    bool markedForDestruction;
} CrushCell;

static CrushCell emptyCell = {0, false, false};

typedef enum {
    BSP_SMALL,
    BSP_MEDIUM,
    BSP_LARGE,
    BSP_CUSTOM,
} BoardSizePreset;

typedef struct {
    BoardSizePreset sizePreset;
    char symbols;
    int width;
    int height;
    int cellCount;
    int score;
    int combo;
    // The list of all tricks in the combo, stuff like "Linear smash", etc.
    char comboTricks[MAX_TRICKS][MAX_TRICK_LENGTH];
    // A malloc-allocated array of all the cells stored in a contiguous way.
    // The array starts with [width] elements of the first row. Then, the next row.
    // Repeat until we get to [height] rows. This is a row-major array.
    // Example :
    // 1 0 1 (index 0-2)
    // 2 4 1 (index 3-5)
    // 0 1 2 (index 6-8)
    // -----
    // This uses C Flexible Array Members to avoid using malloc too much.
    // See https://en.wikipedia.org/wiki/Flexible_array_member to understand how it works (it's easy!).
    CrushCell cells[];
} CrushBoard;

typedef struct {
    int x;
    int y;
} Point;

typedef enum {
    SR_SUCCESS,
    SR_OUT_OF_BOUNDS,
    SR_EMPTY_CELLS,
    SR_NO_MATCH
} SwapResult;

/**
 * Allocates a new board of the given width and height with randomized values,
 * and with the given amount of symbols from 3 to 6. Should be freed with free().
 * @param width the width of the board (> 3)
 * @param height the height of the board (> 3)
 * @return the created crush board
 */
CrushBoard* makeCrushBoard(BoardSizePreset sizePreset, int width, int height, char symbols);

/**
 * Destroys all marked cells.
 * @param board the board
 */
void boardDestroyMarked(CrushBoard* board);

bool boardMarkAlignedCells(CrushBoard* board);

bool boardGravityTick(CrushBoard* board);

SwapResult boardSwapCells(CrushBoard* board, Point posA, Point posB, bool alwaysRevert);

bool boardAnySwapPossible(CrushBoard* board);

void boardResetCombo(CrushBoard* board);

bool boardSaveToFile(CrushBoard* board, const char* path, char errorMessage[256]);

bool boardReadFromFile(const char* path, CrushBoard** outBoard, char errorMessage[256]);

Point boardCellIndexToPos(CrushBoard* board, int index);

/**
 * Finds all the direct neighbors (in a cross shape) of a cell.
 * @param board
 * @param cell
 * @param neighbors
 * @param n
 */
void boardGetNeighbors(CrushBoard* board, Point cell, Point neighbors[], int* n);

void boardGetPresetDimensions(BoardSizePreset preset, int* outWidth, int* outHeight);

// The following functions are inside the header for better performance (inlining).

static bool boardContainsPos(const CrushBoard* board, Point position) {
    return position.x >= 0 && position.y >= 0 && position.x < board->width && position.y < board->height;
}

static bool pointsEqual(Point a, Point b) {
    return a.x == b.x && a.y == b.y;
}

static int loopBackX(CrushBoard* board, int x) {
    if (x >= board->width) {
        return x % board->width;
    } else if (x < 0) {
        x = -x;
        x %= board->width;
        return (board->width - x);
    } else {
        return x;
    }
}

static int loopBackY(CrushBoard* board, int y) {
    if (y >= board->height) {
        return y % board->height;
    } else if (y < 0) {
        y = -y;
        y %= board->height;
        return (board->height - y);
    } else {
        return y;
    }
}

// Temp Debug function
static int boardPosToIndexValidated(const CrushBoard* board, int x, int y, const char* file, int line) {
    if (!boardContainsPos(board, (Point) {x, y})) {
        char buffer[256] = {0};
        sprintf(buffer, "Out of bounds: %s ; %d", file, line);
        RAGE_QUIT(2010, buffer);
    }
    return y * board->width + x;
}

#endif //PROJET_PASTEQUE_BOARD_H
