#ifndef PROJET_PASTEQUE_CRUSH_SCENE_H
#define PROJET_PASTEQUE_CRUSH_SCENE_H

#include "game_state.h"

typedef enum {
    // Click two tiles on the screen to move them.
    MOUSE,
    // Move a virtual cursor using ZQSD keys, and press space to swap with another tile.
    ZQSD_SPACE_CURSOR,
    // Move a virtual cursor using arrow keys (+ numpad), and press enter to swap with another tile.
    ARROWS_ENTER_CURSOR
} CrushInputMethod;

struct CrushData_S;
typedef struct CrushData_S CrushData;

// TODO: Multiple players? Custom input method?
CrushData* makeCrushData();

void crushInit(PastequeGameState* gameState, CrushData* data);
void crushUpdate(PastequeGameState* gameState, CrushData* data, unsigned long deltaTime);
void crushEvent(PastequeGameState* gameState, CrushData* data, Event* pEvent);
void crushDrawBackground(PastequeGameState* gameState, CrushData* data, Screen* pScreen);
void crushDrawForeground(PastequeGameState* gameState, CrushData* data, Screen* pScreen);
void crushFinish(PastequeGameState* gameState, CrushData* data);

#endif //PROJET_PASTEQUE_CRUSH_SCENE_H
