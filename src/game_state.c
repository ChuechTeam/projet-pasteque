#include "game_state.h"
#include <malloc.h>
#include <string.h>
#include "colors.h"
#include "game_state.h"

Panel* gsAddPanel(PastequeGameState* pGameState, int x, int y, int width, int height, PanelAdornment adornment,
                  DrawPanelFunction drawFunc, void* pPanelData) {
    int availableSlot = -1;
    for (int i = 0; i < MAX_PANELS; ++i) {
        if (isEmptyPanel(&pGameState->panels[i])) {
            availableSlot = i;
            break;
        }
    }
    if (availableSlot == -1) {
        RAGE_QUIT(300, "Max number of active panels reached.");
    }
    pGameState->panels[availableSlot] = constructPanel(availableSlot, x, y, width, height,
                                                       adornment, drawFunc, pPanelData, pGameState->screen);
    return &pGameState->panels[availableSlot];
}

void gsRemovePanel(PastequeGameState* pGameState, Panel* panel) {
    if (panel == NULL) {
        RAGE_QUIT(310, "Attempted to remove a NULL Panel.");
    }
    // Make sure our panel pointer is INSIDE the panels array.
    Panel* startPtr = pGameState->panels;
    Panel* endPtr = pGameState->panels + MAX_PANELS;
    if (panel >= startPtr && panel <= endPtr && ((panel - startPtr) % sizeof(Panel)) == 0) {
        freePanelData(panel); // Free any ressources beforehand.
        *panel = emptyPanel;
    } else {
        RAGE_QUIT(311, "Attempted to remove a Panel that doesn't belong to the GameState. (addr: %p)", panel);
    }
}

void gsDrawAllPanels(PastequeGameState* pGameState) {
#if !USE_ERASE
    // We're on the next frame. Transfer the current pixels to the previous pixels, and clear our current pixels.
    memcpy(pGameState->prevFilledPixels, pGameState->curFilledPixels, sizeof(pGameState->curFilledPixels));
    memset(pGameState->curFilledPixels, 0, sizeof(pGameState->curFilledPixels));
#endif
    for (int i = 0; i < MAX_PANELS; ++i) {
        drawPanel(&pGameState->panels[i], pGameState);
    }
#if !USE_ERASE
    // Now, let's compare the pixels of the previous and current frame.
    // If there's a pixel that was FILLED BEFORE and is NOW CLEARED, clear it manually.
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            if (pGameState->prevFilledPixels[y][x] == 1 &&
                pGameState->curFilledPixels[y][x] == 0) {
                // Present before, now gone, REMOVE IT!
                drawLine(pGameState->screen, x, y, 1, ' ', PASTEQUE_COLOR_BLANK);
            }
        }
    }
#endif
}

PastequeGameState* makeGameState() {
    // Allocate a new GameState initialized entirely with zero values.
    PastequeGameState* gs = calloc(1, sizeof(PastequeGameState));
    if (!gs) {
        RAGE_QUIT(320, "Failed to allocate GameState.");
    }
    return gs;
}

void initGameState(PastequeGameState* pGameState, Screen* pScreen) {
    pGameState->screen = pScreen;
}

void gsSwitchScene(PastequeGameState* pGameState, SceneName newScene, void* newSceneData) {
    if (pGameState->currentSceneData) {
        sceneFinish(pGameState->currentScene, pGameState->currentSceneData, pGameState);
        free(pGameState->currentSceneData);
    }
    gsRemoveAllPanels(pGameState);
    pGameState->currentScene = newScene;
    pGameState->currentSceneData = newSceneData;
    sceneInit(newScene, newSceneData, pGameState);
}

void gsRemoveAllPanels(PastequeGameState* pGameState) {
    for (int i = 0; i < MAX_PANELS; ++i) {
        Panel* panel = &pGameState->panels[i];
        if (!isEmptyPanel(panel)) {
            freePanelData(panel);
            *panel = emptyPanel;
        }
    }
}

void gsQuitGame(PastequeGameState* pGameState) {
    pGameState->quitRequested = true;
}
