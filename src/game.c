#include <malloc.h>
#include "libGameRGR2.h"
#include <stdio.h>

typedef struct {
    int hello;
} PastequeGameState;

// Called once at the very start
void init(void* pUserData, Screen* pScreen) {
    setColor(1, 178, 30, 20);
}

// Called everytime we receive an event, such as a key press.
void event(void* pUserData, Screen* pScreen, Event* pEvent) {

}

// Called every tick to update the game state.
int update(void* pUserData, Screen* pScreen, unsigned long deltaTime) {
    return 0; // Continue
}

// Called just after update to draw on the screen.
void draw(void* pUserData, Screen* pScreen) {
    drawLine(pScreen, 1, 1, 5, 'X', 1);
}

// Called just before the game ends.
void finish(void* pUserData) {

}

int main() {
    printf("Hello there!\n");
    Callbacks cb;
    cb.cbDraw = &draw;
    cb.cbEvent = &event;
    cb.cbUpdate = &update;
    cb.cbInit = &init;
    cb.cbFinish = &finish;
    PastequeGameState* gs = malloc(sizeof(PastequeGameState));
    GameData* game = createGame(20, 20, gs, &cb, 1);
    gameLoop(game);
    free(gs);
}