/**
 * board.h
 * --------
 * Contains the core mechanics of the crush game:
 * - cell destruction in horizontal/vertical lines (+ crossing borders)
 * - random generation of boards, without any matching line
 * - gravity (upwards)
 * - cell swapping and validation (i.e. is there any swap possible?)
 * - score and combo system
 * - save/load to file
 */
#ifndef PROJET_PASTEQUE_BOARD_H
#define PROJET_PASTEQUE_BOARD_H

#include <stdbool.h>

#define BOARD_WIDTH_MAX 100
#define BOARD_HEIGHT_MAX 100
#define MAX_SYMBOLS 6
#define MAX_TRICKS 50
#define MAX_TRICK_LENGTH 72

// Helper macros for manipulating the cells array.

// Returns the cell at location (x, y) in the board. Fails if the coordinates are out of bounds.
#define CELL(board, x, y) board->cells[boardPosToIndexValidated(board, x, y, __FILE__, __LINE__)]
// Returns the cell using looping coordinates: width/height modulo is applied onto X/Y.
#define CELL_LP(board, x, y) board->cells[loopBackY(board, y)*board->width + loopBackX(board, x)]
// Same as CELL, but using a Point struct.
#define CELL_PT(board, point) CELL(board, point.x, point.y)

typedef struct {
    // Number from 0 to 6 currently.
    // 0 = empty
    // 1 to 6 = symbols
    char sym;
    bool highlightedNeighbor;
    bool markedForDestruction;
} CrushCell;

static const CrushCell emptyCell = {0, false, false};

typedef enum {
    BSP_SMALL, // 10x7
    BSP_MEDIUM, // 15x10
    BSP_LARGE, // 24x16
    BSP_CUSTOM, // W/H: 3 to 100
} BoardSizePreset;

typedef struct {
    BoardSizePreset sizePreset;
    char symbols; // 4 to 6
    int width; // 3 to 100
    int height; // 3 to 100
    int cellCount; // width*height
    int score;
    int combo;
    long playTime; // Expressed in microseconds
    int storyIndex; // -1 if story mode is disabled (default)
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

// Required for boardSaveFilePath
struct PastequeGameState_S;

// Allocates a new board of the given width and height with randomized values,
// and with the given amount of symbols from 3 to 6. Should be freed with free().
CrushBoard* makeCrushBoard(BoardSizePreset sizePreset, int width, int height, char symbols);

// Destroys all cells with markedForDestruction set to true.
void boardDestroyMarked(CrushBoard* board);

// Sets the markedForDestruction value to true for all cells within a 3+ line.
// Returns true if any cell has been marked, false otherwise.
bool boardMarkAlignedCells(CrushBoard* board);

// Applies upwards-gravity to cells under empty room.
// Returns true if any cell has been moved, false otherwise.
bool boardGravityTick(CrushBoard* board);

// Swaps two cells by enforcing the game rules: a swap cannot be done if it doesn't match a line.
// If alwaysRevert is true, this function won't swap the cells, even if it is a valid swap.
// Returns the result of the swap : success, out of bounds, no match, or empty cell.
SwapResult boardSwapCells(CrushBoard* board, Point posA, Point posB, bool alwaysRevert);

// Returns true if any swap is possible and legal.
bool boardAnySwapPossible(CrushBoard* board);

// Sets the combo to 0 and clears all combo tricks.
void boardResetCombo(CrushBoard* board);

// Returns the path where a save file of name fileName should be stored.
// IMPORTANT: The returned path must be freed using free().
char* boardSaveFilePath(struct PastequeGameState_S* gameState, const char* fileName);

// Saves the board to a file. errorMessage must point to an array of 256 or more characters.
// Returns true if the save succeeded. Else, returns false and fills the errorMessage
// string with the error (usually from the filesystem).
bool boardSaveToFile(CrushBoard* board, const char* path, char errorMessage[256]);

// Reads a save file, and creates a board when successful.
// outBoard will contain a pointer to the parsed board.
// errorMessage must point to an array of 256 or more characters.
//
// Returns true if the parse succeeded. Else, returns false and fills the errorMessage
// string with the error (filesystem or parse error).
bool boardReadFromFile(const char* path, CrushBoard** outBoard, char errorMessage[256]);

// Converts a cell index to (x, y) coordinates.
Point boardCellIndexToPos(CrushBoard* board, int index);

// Finds all the direct neighbors (in a cross shape) of a cell.
void boardGetNeighbors(CrushBoard* board, Point cell, Point neighbors[], int* n);

// Writes the board dimensions of a preset to outWidth and outHeight.
// Doesn't change anything if preset is BSP_CUSTOM.
void boardGetPresetDimensions(BoardSizePreset preset, int* outWidth, int* outHeight);

// The following functions are inside the header for better performance (inlining).

// Returns true if the location exists within the board.
static bool boardContainsPos(const CrushBoard* board, Point position) {
    return position.x >= 0 && position.y >= 0 && position.x < board->width && position.y < board->height;
}

// Returns true if two points are equal.
static bool pointsEqual(Point a, Point b) {
    return a.x == b.x && a.y == b.y;
}

// Applies a modulo operation to the X coordinate, using the board width.
// Example: -1 will become width-1 ; width will become 1
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

// Applies a modulo operation to the Y coordinate, using the board height.
// Example: -1 will become height-1 ; height will become 1
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

// Used in the CELL macro to properly write an error if we're going out of bounds.
static int boardPosToIndexValidated(const CrushBoard* board, int x, int y, const char* file, int line) {
    if (!boardContainsPos(board, (Point) {x, y})) {
        char buffer[256] = {0};
        sprintf(buffer, "Out of bounds: %s ; %d", file, line);
        RAGE_QUIT(2010, buffer);
    }
    return y * board->width + x;
}

#endif //PROJET_PASTEQUE_BOARD_H
