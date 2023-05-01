/**
 * game_state.h
 * -------------
 * Contains the heart of the game. Everything that needs to persist through the entire game belongs
 * to this struct.
 * GameState also manages rendering dirty pixels to avoid flickering.
 */
#ifndef PROJET_PASTEQUE_GAME_STATE_H
#define PROJET_PASTEQUE_GAME_STATE_H
#include "panel.h"

// 16 panels should be enough. It's not like we're making a candy crush battle royale or something.
#define MAX_PANELS 16

// Those won't need to change dynamically. We can get some profit from that (2D arrays).
#define SCREEN_WIDTH 32
#define SCREEN_HEIGHT 18

typedef struct PastequeGameState_S {
    unsigned long hello;

    Panel panels[MAX_PANELS];
    // Convenience pointer to RGR Screen.
    Screen* screen;
    // Value set to 1 if filled, 0 else.
    // This could be optimized to be a bitset but COME ON this is already complex enough...
    char prevFilledPixels[SCREEN_HEIGHT][SCREEN_WIDTH];
    char curFilledPixels[SCREEN_HEIGHT][SCREEN_WIDTH];
} PastequeGameState;

PastequeGameState* makeGameState();
void initGameState(PastequeGameState* pGameState, Screen* pScreen);

Panel* gsAddPanel(PastequeGameState* pGameState, int x, int y, int width, int height,
               PanelAdornment adornment, DrawPanelFunction drawFunc, void* pPanelData, Screen* pScreen);

void gsRemovePanel(PastequeGameState* pGameState, Panel* panel);

void gsDrawAllPanels(PastequeGameState* pGameState);

#endif //PROJET_PASTEQUE_GAME_STATE_H
