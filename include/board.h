#ifndef PROJET_PASTEQUE_BOARD_H
#define PROJET_PASTEQUE_BOARD_H

#include <stdbool.h>

#define MAX_SYMBOLS 6

// Helper macros for manipulating the cells array.
#define CELL(board, x, y) board->cells[boardPosToIndexValidated(board, x, y)]
#define CELL_PT(board, point) CELL(board, point.x, point.y)


typedef struct {
    // Number from 0 to 5 currently.
    // 0 = empty
    char sym;
    bool highlightedNeighbor;
    bool markedForDestruction;
} CrushCell;

static CrushCell emptyCell = {0, false};

typedef struct {
    int width;
    int height;
    int cellCount;
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
typedef Point Coos;

/**
 * Allocates a new board of the given width and height with randomized values,
 * and with the given amount of symbols from 3 to 6. Should be freed with free().
 * @param width the width of the board (> 3)
 * @param height the height of the board (> 3)
 * @return the created crush board
 */
CrushBoard* makeCrushBoard(int width, int height, char symbols);

/**
 * Destroys all marked cells.
 * @param board the board
 */
void boardDestroyMarked(CrushBoard* board);

bool boardMarkAlignedCells(CrushBoard* board, int* score);

bool boardGravityTick(CrushBoard* board);

bool boardSwapCells(CrushBoard* board, Point cellA, Point cellB);

Point boardCellIndexToPos(CrushBoard* board, int index);

/**
 * Finds all the direct neighbors (in a cross shape) of a cell.
 * @param board
 * @param cell
 * @param neighbors
 * @param n
 */
void boardGetNeighbors(CrushBoard* board, Point cell, Point neighbors[], int* n);

// The following functions are inside the header for better performance (inlining).

static bool boardContainsPos(const CrushBoard* board, Point position) {
    return position.x >= 0 && position.y >= 0 && position.x < board->width && position.y < board->height;
}

static bool pointsEqual(Point a, Point b) {
    return a.x == b.x && a.y == b.y;
}

// Temp Debug function
static int boardPosToIndexValidated(const CrushBoard* board, int x, int y) {
    if (!boardContainsPos(board, (Point){x, y})) {
        RAGE_QUIT(2010, "WHAT??? that's not correct...");
    }
    return y*board->width + x;
}

#endif //PROJET_PASTEQUE_BOARD_H
