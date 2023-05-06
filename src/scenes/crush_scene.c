#include "scenes/crush_scene.h"
#include <stdlib.h>
#include "colors.h"
#include <stdbool.h>

#define MAX_WIDTH 30
#define MAX_HEIGHT 30

// Helper macros for manipulating the cells array.
#define CELL_INDEX(data, x, y) (y*data->width + x)
#define CELL(data, x, y) data->cells[y*data->width + x]

// A simple 2D vector
typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    char ch;
} CrushCell;

// Temporary struct until we get the real board logic going on.
struct CrushData_S {
    int width;
    int height;
    int cellCount;
    // A malloc-allocated array of all the cells stored in a contiguous way.
    // The array starts with [width] elements of the first row. Then, the next row.
    // Repeat until we get to [height] rows. This is a row-major array.
    // Note: C Flexible Array Members could be used for this.
    CrushCell* cells;

    Panel* boardPanel;
    Point cursor;
    bool cursorEnabled;
    CrushInputMethod inputMethod;
};

CrushData* makeCrushData(int width, int height, CrushInputMethod inputMethod) {
    if (width < 3 || height < 3 || width > MAX_WIDTH || height > MAX_HEIGHT) {
        RAGE_QUIT(2000, "Invalid Crush array dimensions (width, height) = (%d, %d)", width, height);
    }
    CrushData* data = calloc(1, sizeof(CrushData));
    if (data == NULL) {
        RAGE_QUIT(2001, "Failed to allocate CrushData.");
    }
    data->width = width;
    data->height = height;
    data->cellCount = width * height;

    size_t arraySize = sizeof(CrushCell) * data->cellCount;
    data->cells = calloc(1, arraySize);
    if (data->cells == NULL) {
        RAGE_QUIT(2002, "Failed to allocate CrushData's cell array (is it too big? size: %d).", arraySize);
    }

    data->cursorEnabled = (inputMethod & (CIM_ARROWS_ENTER_CURSOR | CIM_ZQSD_SPACE_CURSOR)) != 0;
    data->inputMethod = inputMethod;
    return data;
}

static Point cellPosition(const CrushData* data, int cellIndex) {
    Point pos = {.x = cellIndex % data->width, .y = cellIndex / data->width};
    return pos;
}

static bool pointsEqual(Point a, Point b) {
    return a.x == b.x && a.y == b.y;
}

void drawBoardPanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    CrushData* data = panelData;
    for (int c = 0; c < data->cellCount; c++) {
        CrushCell* cell = &data->cells[c];
        Point pos = cellPosition(data, c);
        ColorId color = pointsEqual(data->cursor, pos) && data->cursorEnabled ? PASTEQUE_COLOR_YELLOW_HIGHLIGHT
                                                                              : PASTEQUE_COLOR_WHITE;

        panelDrawLine(panel, 2 * pos.x, pos.y, 1, cell->ch, color);
    }
}

void moveCursorDelta(CrushData* data, int deltaX, int deltaY) {
    data->cursor.x += deltaX;
    data->cursor.y += deltaY;

    if (data->cursor.x < 0) { data->cursor.x = 0; }
    if (data->cursor.x >= data->width) { data->cursor.x = data->width - 1; }

    if (data->cursor.y < 0) { data->cursor.y = 0; }
    if (data->cursor.y >= data->height) { data->cursor.y = data->height - 1; }
}

void crushInit(PastequeGameState* gameState, CrushData* data) {
    for (int c = 0; c < data->cellCount; c++) {
        data->cells[c].ch = (char) ('A' + rand() % 3);
    }
    // Leave some space for horizontal spaces.
    data->boardPanel = gsAddPanel(gameState, 2, 2, 2 * data->width, data->height, noneAdornment, &drawBoardPanel, data);
}

void crushUpdate(PastequeGameState* gameState, CrushData* data, unsigned long deltaTime) {

}

void crushEvent(PastequeGameState* gameState, CrushData* data, Event* pEvent) {
    // Move the cursor according to input method settings
    const bool isZQSD = (data->inputMethod & CIM_ZQSD_SPACE_CURSOR) != 0;
    const bool isArrows = (data->inputMethod & CIM_ARROWS_ENTER_CURSOR) != 0;
    const KeyCode code = pEvent->code;

    if (isZQSD && code == KEY_Z || isArrows && code == KEY_ARROW_UP) {
        moveCursorDelta(data, 0, -1);
    } else if (isZQSD && code == KEY_S || isArrows && code == KEY_ARROW_DOWN) {
        moveCursorDelta(data, 0, 1);
    } else if (isZQSD && code == KEY_Q || isArrows && code == KEY_ARROW_LEFT) {
        moveCursorDelta(data, -1, 0);
    } else if (isZQSD && code == KEY_D || isArrows && code == KEY_ARROW_RIGHT) {
        moveCursorDelta(data, 1, 0);
    }
}

void crushDrawBackground(PastequeGameState* gameState, CrushData* data, Screen* pScreen) {
}

void crushDrawForeground(PastequeGameState* gameState, CrushData* data, Screen* pScreen) {

}

void crushFinish(PastequeGameState* gameState, CrushData* data) {

}
