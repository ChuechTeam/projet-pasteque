#include "game_state.h"
#include <malloc.h>
#include <string.h>
#include "colors.h"
#include "game_state.h"
#include <stdbool.h>
#include <stdlib.h>

// This internal function sorts the panelSorted array using insert sort.
// Insert sort is the perfect match for this scenario, as panels are very rarely
// out-of-order. Which means that we'll often have a complexity near O(n) rather than O(n^2).
// It's also easy to implement, which leaves less room for error.
void updatePanelSortedArray(PastequeGameState* pGameState) {
    // Remember: panelSorted is an array with holes (empty panels).
    // However, we can ignore that, since empty panels are always panels on layer 0.
    // Layer 0 is the most common layer in the game anyway. Once a panel gets added,
    // it will take place in the empty panel slot which is *most likely* layer 0.
    for (int i = 1; i < MAX_PANELS; ++i) {
        bool backwards;
        do {
            Panel* prev = pGameState->panelsSorted[i - 1];
            Panel* cur = pGameState->panelsSorted[i];

            if (cur->layer < prev->layer) {
                // Anomaly detected! Swap until we get to a correct order.
                backwards = true;

                pGameState->panelsSorted[i - 1] = cur;
                pGameState->panelsSorted[i] = prev;
                i--;
            } else {
                // We're in the right order. Stop here.
                backwards = false;
            }
        } while (backwards && i > 0);
    }
}

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
    updatePanelSortedArray(pGameState);
    return &pGameState->panels[availableSlot];
}

void gsMovePanelLayer(PastequeGameState* pGameState, Panel* pPanel, int newLayer) {
    pPanel->layer = newLayer;
    updatePanelSortedArray(pGameState);
}

void gsRemovePanel(PastequeGameState* pGameState, Panel* panel) {
    if (panel == NULL) {
        RAGE_QUIT(310, "Attempted to remove a NULL Panel.");
    }
    // Make sure our panel pointer is INSIDE the panels array.
    Panel* startPtr = pGameState->panels;
    Panel* endPtr = pGameState->panels + MAX_PANELS;
    if (panel >= startPtr && panel <= endPtr && ((panel - startPtr) % sizeof(Panel)) == 0) {
        *panel = emptyPanel;
        // Removing a panel doesn't change the order of panels, so no sorting needed.
    } else {
        RAGE_QUIT(311, "Attempted to remove a Panel that doesn't belong to the GameState. (addr: %p)", panel);
    }
}

void gsDrawAllPanels(PastequeGameState* pGameState) {
    for (int i = 0; i < MAX_PANELS; ++i) {
        drawPanel(pGameState->panelsSorted[i], pGameState);
    }
}

PastequeGameState* makeGameState() {
    // Allocate a new GameState initialized entirely with zero values.
    PastequeGameState* gs = calloc(1, sizeof(PastequeGameState));
    if (!gs) {
        RAGE_QUIT(320, "Failed to allocate GameState.");
    }
    for (int i = 0; i < MAX_PANELS; ++i) {
        gs->panelsSorted[i] = &gs->panels[i];
    }
    return gs;
}

void initGameState(PastequeGameState* pGameState, Screen* pScreen) {
    pGameState->screen = pScreen;

    // Turn dynamic colors off if wanted.
    char* dynEnv = getenv("PASTEQUE_NO_DYN_COLORS");
    if (dynEnv != NULL && strcmp(dynEnv, "1") == 0) {
        pGameState->enableDynamicColors = false;
    } else {
        pGameState->enableDynamicColors = true;
    }

    // Enable cheats if wanted.
    char* cheatEnv = getenv("PASTEQUE_CHEAT");
    if (cheatEnv != NULL && strcmp(cheatEnv, "1") == 0) {
        pGameState->enableCheats = true;
    } else {
        pGameState->enableCheats = false;
    }

    // Enable local save files if wanted.
    char* localSaveEnv = getenv("PASTEQUE_LOCAL_SAVE");
    if (localSaveEnv != NULL && strcmp(localSaveEnv, "1") == 0) {
        pGameState->enableLocalSaveFile = true;
    } else {
        pGameState->enableLocalSaveFile = false;
    }
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
            *panel = emptyPanel;
        }
    }
}

void gsQuitGame(PastequeGameState* pGameState) {
    pGameState->quitRequested = true;
}
