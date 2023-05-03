#include "scenes/crush_scene.h"
#include <stdlib.h>
#include "colors.h"

struct CrushData_S {
    int nothing;
};

CrushData* makeCrushData() {
    CrushData* data = calloc(1, sizeof(CrushData));
    if (!data) {
        RAGE_QUIT(2000, "Failed to allocate CrushData.");
    }
    return data;
}

void crushInit(PastequeGameState* gameState, CrushData* data) {

}

void crushUpdate(PastequeGameState* gameState, CrushData* data, unsigned long deltaTime) {

}

void crushEvent(PastequeGameState* gameState, CrushData* data, Event* pEvent) {

}

void crushDrawBackground(PastequeGameState* gameState, CrushData* data, Screen* pScreen) {
    drawText(pScreen, 2, 4, "y'a pas de jeu lol je t'ai bien eu", PASTEQUE_COLOR_WHITE);
}

void crushDrawForeground(PastequeGameState* gameState, CrushData* data, Screen* pScreen) {

}

void crushFinish(PastequeGameState* gameState, CrushData* data) {

}
