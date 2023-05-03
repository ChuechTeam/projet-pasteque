/**
 * game_state.h
 * -------------
 * Contains the heart of the game. Everything that needs to persist through the entire game belongs
 * to this struct. Mainly, GameState contains the current scene the game is rendering.
 * GameState also manages rendering dirty pixels to avoid flickering.
 */
#ifndef PROJET_PASTEQUE_GAME_STATE_H
#define PROJET_PASTEQUE_GAME_STATE_H
#include "panel.h"
#include "scene.h"

// 16 panels should be enough. It's not like we're making a candy crush battle royale or something.
#define MAX_PANELS 16

// Those won't need to change dynamically. We can get some profit from that (2D arrays).
#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 54
#define USE_ERASE 1

typedef struct PastequeGameState_S {
    unsigned long gameTime;

    Panel panels[MAX_PANELS];
    // Convenience pointer to RGR Screen.
    Screen* screen;
    SceneName currentScene;
    void* currentSceneData;
    bool quitRequested;

    // Previous/current filled pixels
    // --------
    // Those arrays are "screenshots" of all the pixels on a frame.
    // Each pixel is either drawn (1) or cleared (0).
    // Using those arrays, we can circumvent the issue of pixels that
    // were drawn before, and now need to be cleared.
    // TODO: Remove this? Is it useless now that erase() does the job easily?

    // Value set to 1 if filled, 0 else.
    // This could be optimized to be a bitset but COME ON this is already complex enough...
    char prevFilledPixels[SCREEN_HEIGHT][SCREEN_WIDTH];
    char curFilledPixels[SCREEN_HEIGHT][SCREEN_WIDTH];
} PastequeGameState;

/**
 * Allocates a new GameState.
 * @return a fresh GameState
 */
PastequeGameState* makeGameState();
void initGameState(PastequeGameState* pGameState, Screen* pScreen);

Panel* gsAddPanel(PastequeGameState* pGameState, int x, int y, int width, int height,
               PanelAdornment adornment, DrawPanelFunction drawFunc, void* pPanelData);

void gsRemovePanel(PastequeGameState* pGameState, Panel* panel);

void gsRemoveAllPanels(PastequeGameState* pGameState);

void gsDrawAllPanels(PastequeGameState* pGameState);

void gsSwitchScene(PastequeGameState* pGameState, SceneName newScene, void* newSceneData);

void gsQuitGame(PastequeGameState* pGameState);

#endif //PROJET_PASTEQUE_GAME_STATE_H
