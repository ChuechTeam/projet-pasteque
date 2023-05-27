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

/**
 * The name of a scene. This enum contains all scenes possible in the game.
 */
typedef enum {
    // Only used for debugging (when the game state is initialized)
    SN_NONE,
    // The main menu of the game
    SN_MAIN_MENU,
    // The actual game! With stuff to crush and all that good stuff!
    SN_CRUSH,
    // The story mode (lol)
    SN_STORY
} SceneName;

struct PastequeGameState_S; // Forward declare the GameState to avoid circular dependency issues

/**
 * Calls the sceneInit function for the current scene.
 *
 * The sceneInit function is called right after switching to a new scene. It can be used
 * to initialize stuff, mainly panels (using gsAddPanel).
 * @param sceneName the name of the scene to initialize
 * @param sceneData the data associated with the scene
 * @param gameState the game state
 */
void sceneInit(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState);

/**
 * Calls the sceneEvent function for the current scene.
 *
 * The sceneEvent function is called once input has been detected, before sceneUpdate.
 * Currently, only keyboard input is supported.
 * @param sceneName the name of the scene to initialize
 * @param sceneData the data associated with the scene
 * @param gameState the game state
 * @param event the input event we received
 */
void sceneEvent(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState, Event* event);

/**
 * Calls the sceneUpdate function for the current scene.
 *
 * The sceneUpdate function is called once every frame, with the game being capped at 60 FPS.
 * The deltaSeconds parameter can be used to know how much time (in microseconds)
 * has elapsed between the previous and current frame, as the 60 FPS timing is not a guarantee.
 *
 * @param sceneName the name of the scene to initialize
 * @param sceneData the data associated with the scene
 * @param gameState the game state
 * @param deltaSeconds the microseconds elapsed between the previous and current frame
 */
void sceneUpdate(SceneName sceneName, void* sceneData,
                 struct PastequeGameState_S* gameState, unsigned long deltaSeconds);

/**
 * Calls the sceneFinish function for the current scene.
 *
 * The sceneFinish function is called just before switching to another scene, or before quitting the game.
 * Currently, it must not be used to free the scene data, as the game state already does that.
 * @param sceneName the name of the scene to initialize
 * @param sceneData the data associated with the scene
 * @param gameState the game state
 */
void sceneFinish(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState);

#endif //PROJET_PASTEQUE_SCENE_H
