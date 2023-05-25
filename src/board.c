#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "libGameRGR2.h"
#include "board.h"

#define LINE_END_AUTODETECT (-1000)

// Define those earlier so we can use them in makeCrushBoard.
bool lineContinues(CrushBoard* board, int x, int y, int offsetX, int offsetY, int lineEnd);

void fillBoardRandom(CrushBoard* board, char symbols);

void fillCellRandom(CrushBoard* board, char symbols, int x, int y, char additionalForbiddenSymbol);

CrushBoard* makeCrushBoard(BoardSizePreset sizePreset, int width, int height, char symbols) {
    if (sizePreset == BSP_CUSTOM && (width < 3 || height < 3 || width > BOARD_WIDTH_MAX || height > BOARD_HEIGHT_MAX)) {
        RAGE_QUIT(2000, "Invalid Crush array dimensions (width, height) = (%d, %d)", width, height);
    }
    if (symbols < 4 || symbols > 6) {
        RAGE_QUIT(2020, "Symbol count invalid (symbols=%d)", symbols);
    }
    int cellCount = width * height;
    CrushBoard* board = calloc(1, sizeof(CrushBoard) + sizeof(CrushCell) * cellCount);
    if (board == NULL) {
        RAGE_QUIT(2001, "Failed to allocate CrushData.");
    }
    board->sizePreset = sizePreset;
    board->symbols = symbols;
    if (sizePreset == BSP_CUSTOM) {
        board->height = height;
        board->width = width;
    } else {
        // Enforce the preset dimensions.
        boardGetPresetDimensions(sizePreset, &board->width, &board->height);
    }
    board->cellCount = cellCount;
    board->score = 0;
    board->combo = 0;
    // data->cells has already been initialized since we've allocated enough space using malloc.

    // Fill with random values
    fillBoardRandom(board, symbols);
    return board;
}

void fillBoardRandom(CrushBoard* board, char symbols) {
    // We need to fill the board without any 3+ match.
    // We're going to fill row by row, left to right.
    // To avoid line matches, we can simply exclude any symbol that will
    // make a line (horizontally or vertically) from the symbol pool.
    // We only need to check left horizontal lines, and top vertical lines,
    // because we're going left to right.
    // We also need to avoid matching lines across borders. This situation should
    // only be checked for the rightmost or bottommost cells.
    // The worst possible generation for both situations is the following:
    // ┌─────────┐
    // │        c│
    // │        c│
    // │        d│
    // │        d│
    // │a a b b x│
    // └─────────┘
    //
    // This leaves us with at most 4 excluded symbols, which is problematic
    // in case the user chooses 4 symbols only!
    // To fix this very specific problem, we can rewind one cell behind
    // and pick another symbol. It is guaranteed for the previous cell to
    // have at least two possible symbols, as:
    // - for this situation to happen, it *has* to be preceded with the same symbol
    // - therefore, there's no forbidden symbol due to a horizontal line (+1 symbol)
    // - there's also no forbidden symbol for horizontal lines crossing the borders (+1 symbol)
    //
    // See the "b" preceding the "x" cell in the example above to understand this.
    //
    // In the fillCellRandom function, the additionalForbiddenSymbol parameter is used to exclude
    // the problematic symbol for the previous cell. We know that there's no forbidden symbol
    // due to a horizontal crossing line, so we can override it and treat it as an "additional" rule.

    for (int y = 0; y < board->height; ++y) {
        for (int x = 0; x < board->width; ++x) {
            fillCellRandom(board, symbols, x, y, 0);
        }
    }
}

void fillCellRandom(CrushBoard* board, char symbols, int x, int y, char additionalForbiddenSymbol) {
    // The four forbidden symbols.
    char forbiddenSymbol1 = 0; // Horizontal line
    char forbiddenSymbol2 = 0; // Vertical line
    char forbiddenSymbol3 = additionalForbiddenSymbol; // Horizontal crossing line (can be overriden)
    char forbiddenSymbol4 = 0; // Vertical crossing line
    int numForbiddenSymbols = 0;

    // Check if we have two consecutive symbols in the horizontal line behind.
    if (x > 1 && lineContinues(board, x - 1, y, -1, 0, LINE_END_AUTODETECT)) {
        forbiddenSymbol1 = CELL(board, x - 1, y).sym;
        numForbiddenSymbols++;
    }

    // Check if we have two consecutive symbols in the vertical line above.
    if (y > 1 && lineContinues(board, x, y - 1, 0, -1, LINE_END_AUTODETECT)) {
        forbiddenSymbol2 = CELL(board, x, y - 1).sym;
        numForbiddenSymbols++;
    }

    // Check if we have a symbol that might induce a horizontal crossing line.
    // Example cases:
    // - A A B B X  (A is a forbidden symbol for X)
    // - A B B A X  (A is a forbidden symbol for X)
    if (x == board->width - 1 && forbiddenSymbol3 == 0) {
        CrushCell firstCell = CELL(board, 0, y);
        CrushCell prevCell = CELL(board, x - 1, y);

        // Case A A B B X  then  A B B A X
        if (lineContinues(board, 0, y, 1, 0, LINE_END_AUTODETECT) || firstCell.sym == prevCell.sym) {
            forbiddenSymbol3 = firstCell.sym;
            numForbiddenSymbols++;
        }
    }

    // Check if we have a symbol that might induce a vertical crossing line.
    // Cases are same as for horizontal, but vertical, you get it.
    if (y == board->height - 1) {
        CrushCell firstCell = CELL(board, x, 0);
        CrushCell prevCell = CELL(board, x, y - 1);

        // Case A A B B X  then  A B B A X
        if (lineContinues(board, x, 0, 0, 1, LINE_END_AUTODETECT) || firstCell.sym == prevCell.sym) {
            forbiddenSymbol4 = firstCell.sym;
            numForbiddenSymbols++;
        }
    }

    char chosenSymbol;
    if (numForbiddenSymbols == 0) {
        // No forbidden symbols. Use random directly.
        chosenSymbol = (char) (1 + (rand() % symbols));
    } else if (numForbiddenSymbols == 4 && symbols == 4) {
        // EDGE CASE! See my essay in the fillBoardRandom function to learn more.

        // forbiddenSymbol1 is the previous cell symbol.
        fillCellRandom(board, symbols, x - 1, y, forbiddenSymbol1);

        // Restart the current function.
        fillCellRandom(board, symbols, x, y, 0);
        return;
    } else {
        // We have forbidden symbols. Prepare a pool of symbols to get true randomness.
        const int poolSize = symbols - numForbiddenSymbols;
        char symbolPool[MAX_SYMBOLS];
        int poolIndex = 0;
        for (char sym = 1; sym <= symbols; ++sym) {
            if (sym != forbiddenSymbol1 && sym != forbiddenSymbol2 &&
                sym != forbiddenSymbol3 && sym != forbiddenSymbol4) {
                symbolPool[poolIndex] = sym;
                poolIndex++;
            }
        }

        chosenSymbol = symbolPool[rand() % poolSize];
    }

    // Finally, fill the cell with the symbol!
    CELL(board, x, y).sym = chosenSymbol;
}

void boardDestroyMarked(CrushBoard* board) {
    if (board == NULL) {
        RAGE_QUIT(2002, "Crush board is NULL.");
    }

    for (int i = 0; i < board->cellCount; i++) {
        if (board->cells[i].markedForDestruction) {
            board->cells[i] = emptyCell;
        }
    }
}

bool boardGravityTick(CrushBoard* board) {
    if (board == NULL) {
        RAGE_QUIT(2002, "Crush board is NULL.");
    }

    bool anyCellMoved = false;

    // Traverse all columns, from top to bottom.
    for (int x = 0; x < board->width; ++x) {
        bool shiftCells = false;

        for (int y = 0; y < board->height - 1; ++y) {
            // Trigger cell shifting when we have an empty cell
            if (!shiftCells && CELL(board, x, y).sym == 0) {
                shiftCells = true;
            }

            if (shiftCells) {
                // Replace with the cell underneath
                CrushCell* cur = &CELL(board, x, y);
                CrushCell* underneath = &CELL(board, x, y + 1);

                // Make sure we don't report that a cell has moved
                // when everything on the bottom is just empty space.
                bool bothEmpty = cur->sym == 0 && underneath->sym == 0;
                if (!bothEmpty) {
                    *cur = *underneath;
                    anyCellMoved = true;
                }
            }
        }

        if (shiftCells) {
            // Empty out the cell on the bottom so we don't get duplicated cells.
            CELL(board, x, board->height - 1) = emptyCell;
        }
    }

    return anyCellMoved;
}

int basePoints(int length) {
    switch (length) {
        case 3:
            return 300;
        case 4:
            return 450;
        case 5:
            return 650;
        case 6:
            return 900;
        case 7:
            return 1500;
        case 8:
            return 2500;
        case 9:
            return 4000;
        case 10:
            return 10000;
        default:
            return 20000 * length;
    }
}

int points(int length, int combo) {
    int base = basePoints(length);
    // Current combo formula: +20% points per combo
    float mult = 1.0f + (combo * 0.2f);
    return (int) (base * mult); // Round down
}

void addLineScore(CrushBoard* board, int length) {
    static const char* lengthPrefixes[] = {
            "%d-Sized",
            "%d-Scaled",
            "%d Long",
            "Huge %d",
            "Gigantic %d"
    };
    static const int lengthPrefixesNum = 5;
    static const char* adjectives[] = {
            "Linear",
            "Sequential",
            "Straight",
            "Sequential",
            "Aligned",
            "Phenomenal",
            "Top-Notch",
            "Chaotic",
            "360°",
            "Cataclysmic",
            "Devastating",
            "Ground-breaking",
            "Astonishing",
            "Marvelous",
            "Mathematically Correct",
            "WTF?!"
    };
    static const int adjectivesNum = 15;
    static const char* nouns1[] = {
            "Line",
            "Sequence",
            "Alignment",
            "Flip",
            "Flick",
            "Swap",
            "No Ruler",
            "WTF?!"
    };
    static const int nouns1Num = 8;
    static const char* nouns2[] = {
            "Removal",
            "Clean-up",
            "Smash",
            "Erasure",
            "Destruction",
            "Carnage",
            "Obliteration",
            "Elimination",
            "Annihilation"
    };
    static const int nouns2Num = 9;

    int pts = points(length, board->combo);
    board->score += pts;
    board->combo++;

    // Now, let's find a FUNNY trick name to amuse our players!
    int trickIndex = board->combo - 1;
    if (trickIndex >= MAX_TRICKS) {
        // Override other tricks on the start of the array.
        // And make sure the string is empty.
        trickIndex = trickIndex % MAX_TRICKS;
        memset(board->comboTricks[trickIndex], 0, sizeof(char) * MAX_TRICK_LENGTH);
    }
    int extraCells = length - 3;
    // Chances are in 1/1000 format.
    int lengthPrefixChance = 150 + extraCells * 600;
    int adjectiveChance = 400 + extraCells * 350;
    int twoNounsChance = 200 + extraCells * 350;

    // Setup the start indices so the more interesting plays get more interesting
    // strings.
    int lpIdx = extraCells >= 3 ? 3 : 0 + extraCells;
    int adjIdx = extraCells >= 3 ? 6 : 2 * extraCells;
    int n1Idx = 0;
    int n2Idx = extraCells >= 2 ? 2 : extraCells;

    char* trickStr = board->comboTricks[trickIndex];

    // Remark: the string should have enough characters for the maximum combination.
    // Currently, it's 64 characters which is more than enough.
    // Step one: the length prefix
    if ((rand() % 1000) < lengthPrefixChance) {
        const char* chosen = lengthPrefixes[lpIdx + (rand() % (lengthPrefixesNum - lpIdx))];
        char formatted[32];
        snprintf(formatted, 32, chosen, length);
        strcat(trickStr, formatted);
        strcat(trickStr, " ");
    }

    // Step two: the adjective
    if ((rand() % 1000) < adjectiveChance) {
        const char* chosen = adjectives[adjIdx + (rand() % (adjectivesNum - adjIdx))];
        strcat(trickStr, chosen);
        strcat(trickStr, " ");
    }

    // Step three: the nouns
    bool bothNouns = (rand() % 1000) < twoNounsChance;
    bool writeNoun1 = bothNouns || ((rand() % 2) == 0);
    bool writeNoun2 = bothNouns || !writeNoun1;
    if (writeNoun1) {
        const char* chosen = nouns1[n1Idx + (rand() % (nouns1Num - n1Idx))];
        strcat(trickStr, chosen);
        strcat(trickStr, " ");
    }
    if (writeNoun2) {
        const char* chosen = nouns2[n2Idx + (rand() % (nouns2Num - n2Idx))];
        strcat(trickStr, chosen);
    }

    // Remove the last space, if any
    size_t trickLen = strlen(trickStr);
    if (trickStr[trickLen - 1] == ' ') {
        trickStr[trickLen - 1] = '\0';
    }

    // Step four (final step): add the amount of points of the line
    char ptsStr[10];
    snprintf(ptsStr, 10, " (+%d)", pts);

    strcat(trickStr, ptsStr);
}

// Returns true if the cell at (x, y) has the same symbol on the next cell.
// The next cell is determined using the offsetX and offsetY parameters. Both cannot be non-zero.
// The finalCellPos parameter can be used to customize the x or y coordinate at which the final cell is.
// (final cell being the very last cell at the end of the line, depending on the direction)
// You can pass LINE_END_AUTODETECT to this parameter to find automatically the end
bool lineContinues(CrushBoard* board, int x, int y, int offsetX, int offsetY, int lineEnd) {
    CrushCell cur = CELL(board, x, y);
    if (cur.sym == 0) {
        // Empty cell, doesn't make lines.
        return false;
    }

    // Find where the last cell is (depending on the direction)
    bool isFinalCell = false;
    if (lineEnd != LINE_END_AUTODETECT) {
        // Use X or Y depending on offsetX or offsetY values.
        isFinalCell = offsetX != 0 ? x == lineEnd : y == lineEnd;
    } else if (offsetX > 0) {
        isFinalCell = (x == board->width - 1);
    } else if (offsetX < 0) {
        isFinalCell = (x == 0);
    } else if (offsetY > 0) {
        isFinalCell = (y == board->height - 1);
    } else if (offsetY < 0) {
        isFinalCell = (y == 0);
    }

    if (isFinalCell) {
        // There's no more next cell, so the line doesn't continue.
        return false;
    }

    CrushCell next = CELL(board, x + offsetX, y + offsetY);

    return cur.sym == next.sym;
}

bool boardMarkAlignedCells(CrushBoard* board) {
    if (board == NULL) {
        RAGE_QUIT(2002, "Crush board is NULL.");
    }

    int length = 1;
    int prevScore = board->score;

    for (int y = 0; y < board->height; y++) {
        CrushCell* firstCell = &CELL(board, 0, y);
        CrushCell* lastCell = &CELL(board, board->width - 1, y);
        int startX = 0;
        int endX = board->width; // Exclusive

        // Handle lines spanning the borders of the screen.
        // Example:
        // A B B A A <- A A A is a line.
        // And reduce the interval of cells to scan to the inner part (in the example, B B).
        if (firstCell->sym != 0 && lastCell->sym != 0 && firstCell->sym == lastCell->sym) {
            length = 2; // First and last cell.

            // Find forward cells A [A A] B B A A A
            // Make sure we don't loop forever if we have a full line,
            // for example, stop here: A A A| A (| is the startX cursor) [startX = 2; length 2 + 2 = 4]
            while (startX < endX - 2) {
                if (lineContinues(board, startX, y, 1, 0, LINE_END_AUTODETECT)) {
                    startX++;
                    length++;
                } else {
                    break;
                }
            }

            // Find previous cells A A A B B [A A] A
            while (endX - 2 > startX) {
                if (lineContinues(board, endX - 1, y, -1, 0, LINE_END_AUTODETECT)) {
                    endX--;
                    length++;
                } else {
                    break;
                }
            }

            // Further delimit the search interval
            // Example: A A A <B B >A A
            // startX (<) = 3
            // endX (>) = 6 (exclusive)
            startX++;
            endX--;

            // Mark the cells for destruction, both from start and end.
            if (length >= 3) {
                for (int xs = 0; xs < startX; xs++) {
                    CELL(board, xs, y).markedForDestruction = true;
                }
                for (int xe = board->width; xe > endX; xe--) {
                    CELL(board, xe - 1, y).markedForDestruction = true;
                }
                addLineScore(board, length);
            }
        }

        // Reset the length
        length = 1;

        for (int x = startX; x < endX; x++) {
            if (lineContinues(board, x, y, 1, 0, endX - 1)) {
                length++;
            } else {
                if (length >= 3) {
                    for (int k = 0; k < length; k++) {
                        CELL(board, x - k, y).markedForDestruction = true;
                    }
                    addLineScore(board, length);
                }
                length = 1;
            }
        }
    }

    //check for columns
    for (int x = 0; x < board->width; x++) {
        CrushCell* firstCell = &CELL(board, x, 0);
        CrushCell* lastCell = &CELL(board, x, board->height - 1);
        int startY = 0;
        int endY = board->height; // Exclusive

        // Handle lines spanning the borders of the screen. (same thing but vertical)
        if (firstCell->sym != 0 && lastCell->sym != 0 && firstCell->sym == lastCell->sym) {
            length = 2;

            // Find forward cells A [A A] B B A A A (vertically)
            // Make sure we don't loop forever if we have a full line,
            // for example, stop here: A A A| A (| is the startY cursor) [startY = 2; length 2 + 2 = 4]
            while (startY < endY - 2) {
                if (lineContinues(board, x, startY, 0, 1, LINE_END_AUTODETECT)) {
                    startY++;
                    length++;
                } else {
                    break;
                }
            }

            // Find previous cells A A A B B [A A] A (vertically)
            while (endY - 2 > startY) {
                if (lineContinues(board, x, endY - 1, 0, -1, LINE_END_AUTODETECT)) {
                    endY--;
                    length++;
                } else {
                    break;
                }
            }

            // Further delimit the search interval
            // Example: A A A <B B >A A
            // startY (<) = 3
            // endY (>) = 6 (exclusive)
            startY++;
            endY--;

            // Mark the cells for destruction, both from start and end.
            if (length >= 3) {
                for (int ys = 0; ys < startY; ys++) {
                    CELL(board, x, ys).markedForDestruction = true;
                }
                for (int ye = board->height; ye > endY; ye--) {
                    CELL(board, x, ye - 1).markedForDestruction = true;
                }
                addLineScore(board, length);
            }

            length = 1;
        }

        for (int y = startY; y < endY; y++) {
            if (lineContinues(board, x, y, 0, 1, endY - 1)) {
                length++;
            } else {
                if (length > 2) {
                    for (int k = 0; k < length; k++) {
                        CELL(board, x, y - k).markedForDestruction = true;
                    }
                    addLineScore(board, length);
                }
                length = 1;
            }
        }
    }

    // If the score has changed, then we have destroyed some cells in the process.
    return prevScore != board->score;
}

// Returns true when replacing the given cell is within a line.
bool cellWithinLine(CrushBoard* board, Point position) {
    if (board == NULL) {
        RAGE_QUIT(2002, "Crush board is NULL.");
    }
    if (!boardContainsPos(board, position)) {
        RAGE_QUIT(2005, "Position out of crush board.");
    }

    int x = position.x;
    int y = position.y;
    // Initialize both to 1 as the symbol already counts in the search.
    int horizLength = 1;
    int vertLength = 1;

    char symbol = CELL(board, x, y).sym;

    // Check for horizontal lines, we only need to check for 3 consecutive symbols.
    // We use the LP (Loop back) macro, so we can search for lines crossing the screen borders,
    // such as A B B A A.

    // Search left
    if (CELL_LP(board, x - 1, y).sym == symbol) {
        horizLength++;
        if (CELL_LP(board, x - 2, y).sym == symbol) {
            horizLength++;
        }
    }

    // Search right
    if (CELL_LP(board, x + 1, y).sym == symbol) {
        horizLength++;
        if (CELL_LP(board, x + 2, y).sym == symbol) {
            horizLength++;
        }
    }
    if (horizLength >= 3) {
        return true;
    }

    // Now check for vertical lines
    // Search up
    if (CELL_LP(board, x, y - 1).sym == symbol) {
        vertLength++;
        if (CELL_LP(board, x, y - 2).sym == symbol) {
            vertLength++;
        }
    }

    // Search down
    if (CELL_LP(board, x, y + 1).sym == symbol) {
        vertLength++;
        if (CELL_LP(board, x, y + 2).sym == symbol) {
            vertLength++;
        }
    }
    if (vertLength >= 3) {
        return true;
    } else {
        return false;
    }
}

Point boardCellIndexToPos(CrushBoard* board, int index) {
    if (board == NULL) {
        RAGE_QUIT(2002, "Crush board is NULL.");
    }

    Point pos = {.x = index % board->width, .y = index / board->width};
    return pos;
}

void boardGetNeighbors(CrushBoard* board, Point cell, Point* neighbors, int* n) {
    if (board == NULL) {
        RAGE_QUIT(2002, "Crush board is NULL.");
    }

    *n = 0;
    Point leftCell = {.x=cell.x - 1, .y=cell.y};
    Point rightCell = {.x=cell.x + 1, .y=cell.y};
    Point topCell = {.x=cell.x, .y=cell.y - 1};
    Point bottomCell = {.x=cell.x, .y=cell.y + 1};

    if (boardContainsPos(board, leftCell)) {
        neighbors[*n] = leftCell;
        (*n)++;
    }
    if (boardContainsPos(board, rightCell)) {
        neighbors[*n] = rightCell;
        (*n)++;
    }
    if (boardContainsPos(board, topCell)) {
        neighbors[*n] = topCell;
        (*n)++;
    }
    if (boardContainsPos(board, bottomCell)) {
        neighbors[*n] = bottomCell;
        (*n)++;
    }
}

SwapResult boardSwapCells(CrushBoard* board, Point posA, Point posB, bool alwaysRevert) {
    if (board == NULL) {
        RAGE_QUIT(2002, "Crush board is NULL.");
    }

    // Make sure the cells are inside the board.
    // We won't validate adjacency for now.
    if (!boardContainsPos(board, posA) || !boardContainsPos(board, posB) || pointsEqual(posA, posB)) {
        return SR_OUT_OF_BOUNDS;
    }

    CrushCell cellA = CELL_PT(board, posA);
    CrushCell cellB = CELL_PT(board, posB);

    // Disallow swapping empty cells.
    if (cellA.sym == 0 || cellB.sym == 0) {
        return SR_EMPTY_CELLS;
    }

    // Swap the cells (can be reverted later)
    CELL_PT(board, posA).sym = cellB.sym;
    CELL_PT(board, posB).sym = cellA.sym;

    // Keep the swap when it actually does a match.
    bool swapSuccessful = cellWithinLine(board, posA) || cellWithinLine(board, posB);
    if (!swapSuccessful || alwaysRevert) {
        // The swap is invalid (or we always want to revert), revert changes.
        CELL_PT(board, posA).sym = cellA.sym;
        CELL_PT(board, posB).sym = cellB.sym;
    }

    return swapSuccessful ? SR_SUCCESS : SR_NO_MATCH;
}

bool boardAnySwapPossible(CrushBoard* board) {
    if (board == NULL) {
        RAGE_QUIT(2002, "Crush board is NULL.");
    }

    // Test all left to right swaps, and top to bottom swaps.
    // Regarding complexity, this function can be really costly: O(WxH)!
    // No need to worry though, since the worst case (no swaps at all) happens
    // when we have a low amount of non-empty cells.

    for (int y = 0; y < board->height; ++y) {
        for (int x = 0; x < board->width; ++x) {
            // Left to right swap
            if (x != board->width - 1) {
                // Use the true parameter in alwaysRevert to not actually swap the cells
                if (boardSwapCells(board, (Point) {x, y}, (Point) {x + 1, y}, true) == SR_SUCCESS) {
                    return true;
                }
            }
            // Top to bottom swap
            if (y != board->height - 1) {
                if (boardSwapCells(board, (Point) {x, y}, (Point) {x, y + 1}, true) == SR_SUCCESS) {
                    return true;
                }
            }
        }
    }

    // No swap found at all.
    return false;
}

void boardGetPresetDimensions(BoardSizePreset preset, int* outWidth, int* outHeight) {
    switch (preset) {
        case BSP_CUSTOM:
            break; // Nothing to set.
        case BSP_SMALL:
            *outWidth = 10;
            *outHeight = 7;
            break;
        case BSP_MEDIUM:
            *outWidth = 15;
            *outHeight = 10;
            break;
        case BSP_LARGE:
            *outWidth = 24;
            *outHeight = 16;
            break;
    }
}

bool boardSaveToFile(CrushBoard* board, const char* path, char errorMessage[256]) {
    if (board == NULL) {
        RAGE_QUIT(2002, "Crush board is NULL.");
    }

    FILE* file = fopen(path, "w");
    if (file == NULL) {
        snprintf(errorMessage, 256, "Échec de l'ouverture du fichier, code d'erreur %d :\n%s\n(Chemin : %s)",
                 errno, strerror(errno), path);
        return false;
    }
    bool success = true;
    success &= fprintf(file, "width=%d\n", board->width) != EOF;
    success &= fprintf(file, "height=%d\n", board->height) != EOF;
    success &= fprintf(file, "preset=%d\n", board->sizePreset) != EOF;
    success &= fprintf(file, "symbols=%hhd\n", board->symbols) != EOF;
    success &= fprintf(file, "score=%d\n", board->score) != EOF;
    success &= fprintf(file, "playtime=%ld\n", board->playTime) != EOF;
    success &= fprintf(file, "cells:\n") != EOF;
    for (int y = 0; y < board->height; ++y) {
        for (int x = 0; x < board->width; ++x) {
            success &= fprintf(file, "%hhd", CELL(board, x, y).sym) != EOF;
            if (x != board->width - 1) {
                success &= fputc(' ', file) != EOF;
            }
        }
        if (y != board->height - 1) {
            success &= fputc('\n', file) != EOF;
        }
    }
    fclose(file);

    if (!success) {
        if (ferror(file)) {
            snprintf(errorMessage, 256,
                     "Échec lors de l'écriture dans le fichier, code d'erreur %d :\n%s\n(Chemin : %s)",
                     errno, strerror(errno), path);
        } else {
            snprintf(errorMessage, 256,
                     "Erreur inconnue lors de l'écriture dans le fichier\n(Chemin : %s)", path);
        }
    }

    return success;
}

bool boardReadFromFile(const char* path, CrushBoard** outBoard, char* errorMessage) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        if (errno == ENOENT) {
            snprintf(errorMessage, 256, "Fichier introuvable\n(Chemin: %s)", path);
        } else {
            snprintf(errorMessage, 256, "Échec de l'ouverture du fichier, code d'erreur %d :\n%s\n(Chemin : %s)",
                     errno, strerror(errno), path);
        }
        return false;
    }

    CrushBoard* createdBoard = NULL;
    int width = -1, height = -1, score = -1;
    char symbols = 0;
    BoardSizePreset preset = -1;
    long playTime = -1;
    bool error = false;
    bool fileError = false;
    bool parseCells = false;
    char line[256];

    while (!error && !parseCells) {
        if (!fgets(line, 256, file)) {
            error = true;
            fileError = true;
        } else if (width == -1 && sscanf(line, "width=%d", &width)) {
            // Parsed.
        } else if (height == -1 && sscanf(line, "height=%d", &height)) {
            // Parsed.
        } else if (score == -1 && sscanf(line, "score=%d", &score)) {
            // Parsed.
        } else if (symbols == 0 && sscanf(line, "symbols=%hhd", &symbols)) {
            // Parsed
        } else if (preset == -1 && sscanf(line, "preset=%d", (int*) &preset)) {
            // Parsed
        } else if (playTime == -1 && sscanf(line, "playtime=%ld", &playTime)) {
            // Parsed
        } else if (strcmp(line, "cells:\n") == 0) {
            parseCells = true;
        } else {
            error = true;
            snprintf(errorMessage, 256, "Ligne invalide");
        }
    }

    // Validate all other values beforehand
    if (!error) {
        if (width < 3 || height < 3 || width > BOARD_WIDTH_MAX || height > BOARD_HEIGHT_MAX) {
            error = true;
            snprintf(errorMessage, 256, "Dimensions invalides (L, H) = (%d, %d)", width, height);
        } else if (score < 0) {
            error = true;
            snprintf(errorMessage, 256, "Score invalide (%d)", score);
        } else if (symbols < 4 || symbols > 6) {
            error = true;
            snprintf(errorMessage, 256, "Nombre de symboles invalide (%hhd)", symbols);
        } else if (playTime < 0) {
            error = true;
            snprintf(errorMessage, 256, "Temps de jeu invalide (%ld)", playTime);
        } else if (preset < BSP_SMALL || preset > BSP_CUSTOM) {
            error = true;
            snprintf(errorMessage, 256, "Taille prédéfinie invalide (%hhd)", preset);
        } else if (preset != BSP_CUSTOM) {
            int presetW, presetH;
            boardGetPresetDimensions(preset, &presetW, &presetH);
            if (width != presetW || height != presetH) {
                error = true;
                snprintf(errorMessage, 256,
                         "Dimensions non correspondantes à la taille prédéfinie (%d, %d) != (%d, %d)",
                         width, height, presetW, presetH);
            }
        }
    }

    // Allocate the crush board
    if (!error) {
        int cellCount = width * height;
        createdBoard = calloc(1, sizeof(CrushBoard) + sizeof(CrushCell) * cellCount);
        if (createdBoard == NULL) {
            error = true;
            snprintf(errorMessage, 256, "Mémoire insuffisante pour créer le tableau.");
        } else {
            createdBoard->width = width;
            createdBoard->height = height;
            createdBoard->sizePreset = preset;
            createdBoard->symbols = symbols;
            createdBoard->score = score;
            createdBoard->cellCount = cellCount;
        }
    }

    int y = 0;
    while (!error && y < height) {
        int x = 0;
        char* cursor = line;

        if (!fgets(line, 256, file)) {
            error = true;
            fileError = true;
        }

        while (!error && x < width) {
            char* prevCursor = cursor;

            long symbol = strtol(cursor, &cursor, 10);
            if (prevCursor == cursor) {
                error = true;
                snprintf(errorMessage, 256, "Impossible de lire un nombre entier dans la grille");
            } else if (symbol < 0 || symbol > 6) {
                error = true;
                snprintf(errorMessage, 256, "Symbole invalide (%ld)", symbol);
            } else {
                CELL(createdBoard, x, y).sym = (char) symbol;
                x++;
            }
        }

        if (!error) {
            y++;
        }
    }

    // Conclude!
    if (!error) {
        *outBoard = createdBoard;
    } else {
        if (createdBoard != NULL) {
            free(createdBoard);
        }
        *outBoard = NULL;

        if (fileError) {
            if (feof(file)) {
                snprintf(errorMessage, 256, "Fin du fichier atteinte prématurément.");
            } else {
                snprintf(errorMessage, 256,
                         "Erreur lors de la lecture du fichier, code d'erreur %d :\n%s\n(Chemin : %s)",
                         errno, strerror(errno), path);
            }
        } else {
            // Error message already filled.
        }
    }

    fclose(file);
    return !error;
}

void boardResetCombo(CrushBoard* board) {
    board->combo = 0;
    // Fill the comboTricks array with zero values.
    memset(board->comboTricks, 0, sizeof(board->comboTricks));
}
