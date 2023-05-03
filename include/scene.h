/**
 * scene.h
 * ------------
 * Splits the game into multiple "scenes" (can also be called "screens", "pages", etc.),
 * with functions corresponding to the usual game loop.
 * Each scene has its own data structure persisted while the scene is displayed on the screen.
 * When adding a new scene, make sure to:
 * - add a enum entry to SceneName
 * - call scene-specific game loop functions in sceneX functions. (See scene.c for an example)
 */
#ifndef PROJET_PASTEQUE_SCENE_H
#define PROJET_PASTEQUE_SCENE_H
#include "libGameRGR2.h"

typedef enum {
    SN_NONE,
    SN_MAIN_MENU,
    SN_CRUSH
} SceneName;

struct PastequeGameState_S; // Forward declare the GameState to avoid circular dependency issues

void sceneInit(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState);

void sceneEvent(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState, Event* event);

void sceneUpdate(SceneName sceneName, void* sceneData,
                 struct PastequeGameState_S* gameState, unsigned long deltaSeconds);

void sceneDrawBackground(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState);
void sceneDrawForeground(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState);

void sceneFinish(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState);

#endif //PROJET_PASTEQUE_SCENE_H
