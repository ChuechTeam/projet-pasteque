#ifndef PROJET_PASTEQUE_CRUSH_SCENE_H
#define PROJET_PASTEQUE_CRUSH_SCENE_H

#include "game_state.h"
#include "board.h"

/**
 * The input settings for a player.
 * This is a flags enum, so multiple input options can be assigned to one player.
 * To check if one input option is enabled, use this code: (myVar & CIM_X) != 0
 */
typedef enum {
    // Click two tiles on the screen to move them.
    CIM_MOUSE = 1,
    // Move a virtual cursor using ZQSD keys, and press space to swap with another tile.
    CIM_ZQSD_SPACE_CURSOR = 2,
    // Move a virtual cursor using arrow keys (+ numpad), and press enter to swap with another tile.
    CIM_ARROWS_ENTER_CURSOR = 4,
    // All input options enabled!
    CIM_ALL = CIM_MOUSE | CIM_ZQSD_SPACE_CURSOR | CIM_ARROWS_ENTER_CURSOR
} CrushInputMethod;

struct CrushData_S; // Defined in crush_scene.c
typedef struct CrushData_S CrushData;

// TODO: Multiple players?
// -1 to storyIndex disables the story
CrushData* makeCrushData(CrushBoard* board, CrushInputMethod inputMethod);

// Shows the next chapter automatically, after a given period of time.
void crushStoryAutoSkip(CrushData* data, long micros);

void crushInit(PastequeGameState* gameState, CrushData* data);

void crushUpdate(PastequeGameState* gameState, CrushData* data, unsigned long deltaTime);

void crushEvent(PastequeGameState* gameState, CrushData* data, Event* pEvent);

void crushFinish(PastequeGameState* gameState, CrushData* data);

#endif //PROJET_PASTEQUE_CRUSH_SCENE_H
