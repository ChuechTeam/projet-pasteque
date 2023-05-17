#include <malloc.h>
#include "libGameRGR2.h"
#include <stdio.h>
#include "game_state.h"
#include "colors.h"
#include "scene.h"
#include "scenes/main_menu_scene.h"
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32

#include <Windows.h>

#endif

// Called once at the very start
void init(void* pUserData, Screen* pScreen) {
    PastequeGameState* gs = pUserData;

    initGameState(gs, pScreen);
    initColors();

    gsSwitchScene(gs, SN_MAIN_MENU, makeMainMenuData());
}

// Called everytime a key is pressed.
void event(void* pUserData, Screen* pScreen, Event* pEvent) {
    PastequeGameState* gs = pUserData;

    // On Windows, PDCurses uses lowercase characters for key events.
    // This conflicts with the usual uppercase characters we get.
    // And this also happens in Linux and this time I have no idea why!
    if (pEvent->code >= 'a' && pEvent->code <= 'z') {
        pEvent->code = (KeyCode) toupper(pEvent->code);
    }

    sceneEvent(gs->currentScene, gs->currentSceneData, gs, pEvent);
}

// Called every tick to update the game state.
// deltaTime is the time elapsed, in microseconds (1ms = 1000µs), between the previous frame and now
int update(void* pUserData, Screen* pScreen, unsigned long deltaTime) {
    PastequeGameState* gs = pUserData;

    gs->gameTime += deltaTime;

    if (gs->quitRequested) {
        return 1; // Stop!
    } else {
        sceneUpdate(gs->currentScene, gs->currentSceneData, gs, deltaTime);
        return 0; // Continue
    }
}

// Called just after update to draw on the screen.
// IMPORTANT: The screen isn't cleared automatically. Any leftover should be cleared manually.
void draw(void* pUserData, Screen* pScreen) {
    PastequeGameState* gs = pUserData;

    erase();
    sceneDrawBackground(gs->currentScene, gs->currentSceneData, gs);
    gsDrawAllPanels(gs);
    sceneDrawForeground(gs->currentScene, gs->currentSceneData, gs);
}

// Called just before the game ends.
void finish(void* pUserData) {
    PastequeGameState* gs = pUserData;

    sceneFinish(gs->currentScene, gs->currentSceneData, gs);
}

int main() {
#ifdef _WIN32
    // Make sure Windows works correctly with UTF8 characters.
    SetConsoleOutputCP(CP_UTF8);
#endif

    srand(time(NULL));
    printf("Projet Pasteque launched!\n");

    Callbacks cb;
    cb.cbDraw = &draw;
    cb.cbEvent = &event;
    cb.cbUpdate = &update;
    cb.cbInit = &init;
    cb.cbFinish = &finish;

    PastequeGameState* gs = makeGameState();
    // The two first parameters are useless now that we use getmaxyx.
    GameData* game = createGame(20, 20, gs, &cb, 1);
    gameLoop(game);
    free(gs);
}