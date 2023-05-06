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
    for (int i = 0; i < MAX_PANELS; ++i) {
        drawPanel(&pGameState->panels[i], pGameState);
    }
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
