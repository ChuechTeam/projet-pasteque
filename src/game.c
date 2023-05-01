#include <malloc.h>
#include "libGameRGR2.h"
#include <stdio.h>
#include "game_state.h"
#include "colors.h"
#ifdef _WIN32
#include <Windows.h>
#endif

Panel* coolPanel;
void drawMyPanel(Panel* pPanel, PastequeGameState* pState, void* sup) {
    panelDrawLine(pPanel, 0, 0, 6, 'A', PASTEQUE_COLOR_WHITE);
    panelDrawLine(pPanel, 1, 1, 4, 'B', PASTEQUE_COLOR_WHITE);
    panelDrawLine(pPanel, 2, 2, 2, 'C', PASTEQUE_COLOR_WHITE);
    panelDrawLine(pPanel, (int)(pState->hello/1000000)%4, 4, 3, 'D', PASTEQUE_COLOR_WHITE);
}

// Called once at the very start
void init(void* pUserData, Screen* pScreen) {
    PastequeGameState* gs = pUserData;
    initGameState(gs, pScreen);
    initColors();

    coolPanel = gsAddPanel(gs, 2, 2, 10, 6, (PanelAdornment){PAS_DOUBLE_BORDER, 2}, &drawMyPanel, NULL, pScreen);
}

// Called everytime a key is pressed.
void event(void* pUserData, Screen* pScreen, Event* pEvent) {

}

// Called every tick to update the game state.
// deltaTime is the time elapsed, in microseconds (1ms = 1000µs), between the previous frame and now
int update(void* pUserData, Screen* pScreen, unsigned long deltaTime) {
    PastequeGameState* gs = pUserData;
    gs->hello+=deltaTime;
    panelTranslate(coolPanel, (int)(gs->hello/100000)%10, 3 + (int)(gs->hello/100000)%2);
    return 0; // Continue
}

// Called just after update to draw on the screen.
// IMPORTANT: The screen isn't cleared automatically. Any leftover should be cleared manually.
void draw(void* pUserData, Screen* pScreen) {
    PastequeGameState* gs = pUserData;
    gsDrawAllPanels(gs);
}

// Called just before the game ends.
void finish(void* pUserData) {

}

int main() {
#ifdef _WIN32
    // Make sure Windows works correctly with UTF8 characters.
    SetConsoleOutputCP(CP_UTF8);
#endif

    Callbacks cb;
    cb.cbDraw = &draw;
    cb.cbEvent = &event;
    cb.cbUpdate = &update;
    cb.cbInit = &init;
    cb.cbFinish = &finish;
    PastequeGameState* gs = makeGameState();
    GameData* game = createGame(SCREEN_WIDTH, SCREEN_HEIGHT, gs, &cb, 1);
    gameLoop(game);
    free(gs);
}