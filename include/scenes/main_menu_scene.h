#ifndef PROJET_PASTEQUE_MAIN_MENU_SCENE_H
#define PROJET_PASTEQUE_MAIN_MENU_SCENE_H

#include "game_state.h"

// Forward declare the MainMenuData struct.
// This allows us to define the struct members into the main_menu_scene.c file.
// This way, recompilation is much faster, as editing this header file would recompile a lot of files.
// Also, it's much more handy to edit stuff in the main_menu_scene.c file than having to manage two files.
struct MainMenuData_S;
typedef struct MainMenuData_S MainMenuData;

MainMenuData* makeMainMenuData();

void mainMenuInit(PastequeGameState* gameState, MainMenuData* data);

void mainMenuUpdate(PastequeGameState* gameState, MainMenuData* data, unsigned long deltaTime);

void mainMenuEvent(PastequeGameState* gameState, MainMenuData* data, Event* pEvent);

void mainMenuDrawBackground(PastequeGameState* gameState, MainMenuData* data, Screen* pScreen);

void mainMenuDrawForeground(PastequeGameState* gameState, MainMenuData* data, Screen* pScreen);

void mainMenuFinish(PastequeGameState* gameState, MainMenuData* data);

#endif //PROJET_PASTEQUE_MAIN_MENU_SCENE_H
