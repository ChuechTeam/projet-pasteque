#include "scene.h"
#include "scenes/main_menu_scene.h"
#include "scenes/crush_scene.h"
#include "scenes/story_scene.h"

void sceneInit(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState) {
    if (sceneName == SN_MAIN_MENU) {
        mainMenuInit(gameState, sceneData);
    } else if (sceneName == SN_CRUSH) {
        crushInit(gameState, sceneData);
    } else if (sceneName == SN_STORY) {
        storyInit(gameState, sceneData);
    }
}

void sceneEvent(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState, Event* event) {
    if (sceneName == SN_MAIN_MENU) {
        mainMenuEvent(gameState, sceneData, event);
    } else if (sceneName == SN_CRUSH) {
        crushEvent(gameState, sceneData, event);
    } else if (sceneName == SN_STORY) {
        storyEvent(gameState, sceneData, event);
    }
}

void sceneUpdate(SceneName sceneName, void* sceneData,
                 struct PastequeGameState_S* gameState, unsigned long deltaSeconds) {
    if (sceneName == SN_MAIN_MENU) {
        mainMenuUpdate(gameState, sceneData, deltaSeconds);
    } else if (sceneName == SN_CRUSH) {
        crushUpdate(gameState, sceneData, deltaSeconds);
    } else if (sceneName == SN_STORY) {
        storyUpdate(gameState, sceneData, deltaSeconds);
    }
}

void sceneDrawBackground(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState) {
    if (sceneName == SN_MAIN_MENU) {
        mainMenuDrawBackground(gameState, sceneData, gameState->screen);
    } else if (sceneName == SN_CRUSH) {
        crushDrawBackground(gameState, sceneData, gameState->screen);
    } else if (sceneName == SN_STORY) {
        storyDrawBackground(gameState, sceneData, gameState->screen);
    }
}

void sceneDrawForeground(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState) {
    if (sceneName == SN_MAIN_MENU) {
        mainMenuDrawForeground(gameState, sceneData, gameState->screen);
    } else if (sceneName == SN_CRUSH) {
        crushDrawForeground(gameState, sceneData, gameState->screen);
    } else if (sceneName == SN_STORY) {
        storyDrawForeground(gameState, sceneData, gameState->screen);
    }
}

void sceneFinish(SceneName sceneName, void* sceneData, struct PastequeGameState_S* gameState) {
    if (sceneName == SN_MAIN_MENU) {
        mainMenuFinish(gameState, sceneData);
    } else if (sceneName == SN_CRUSH) {
        crushFinish(gameState, sceneData);
    } else if (sceneName == SN_STORY) {
        storyFinish(gameState, sceneData);
    }
}
