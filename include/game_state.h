/**
 * game_state.h
 * -------------
 * Contains the heart of the game. Everything that needs to persist through the entire game belongs
 * to this struct. Mainly, GameState contains the current scene the game is rendering, and the panels.
 */
#ifndef PROJET_PASTEQUE_GAME_STATE_H
#define PROJET_PASTEQUE_GAME_STATE_H

#include "panel.h"
#include "scene.h"

// Add the access function for Windows
// From: https://stackoverflow.com/a/230068
#ifdef WIN32

#include <io.h>

#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

// 32 panels should be enough. It's not like we're making a candy crush battle royale or something.
#define MAX_PANELS 32
#define MILLI_IN_MICROS 1000L
#define MICROS(millis) (millis*MILLI_IN_MICROS)
/**
 * The game state, containing all the panels and the current scene.
 */
typedef struct PastequeGameState_S {
    // The amount of microseconds elapsed since the beginning of the game.
    unsigned long gameTime;

    // True if dynamic colors are activated, else fallback to static colors.
    bool enableDynamicColors;
    // True if cheats are enabled (story mode skip, currently)
    bool enableCheats;
    // True to save files to the user's home directory.
    bool enableLocalSaveFile;

    // The array containing all panels. By default, they are zeroed-out so they don't render.
    Panel panels[MAX_PANELS];
    // Contains pointers to the panels array, but sorted by the panel layer
    // Empty panels are ignored.
    Panel* panelsSorted[MAX_PANELS];
    // Convenience pointer to RGR Screen.
    Screen* screen;
    // The name of the current scene
    SceneName currentScene;
    // The scene data associated to that scene.
    void* currentSceneData;
    // Whether the game should quit on the next frame.
    bool quitRequested;
} PastequeGameState;

/**
 * Allocates a new GameState. Must be cleared using free.
 * @return a fresh GameState
 */
PastequeGameState* makeGameState();

/**
 * Initializes the GameState with a Screen from the RGR lib.
 * @param pGameState the GameState to initialize
 * @param pScreen the screen
 */
void initGameState(PastequeGameState* pGameState, Screen* pScreen);

/**
 * Adds a panel to the game, a drawable region of the screen.
 *
 * The created panel will have the given dimensions (x, y, width, height), and an optional
 * adornment rendered outside the panel: use noneAdornment for having no adornment.
 *
 * The draw function (drawFunc) is called every frame to draw the contents of the panel,
 * it cannot not be NULL. Inside, you can use panel draw functions such as panelDrawText.
 *
 * An optional panel data pointer (pPanelData) can be added to access additional data inside
 * the draw function. It is not freed once the panel is destroyed.
 *
 * The returned panel belongs to the GameState, and should *not* be freed. It is destroyed
 * automatically when gsRemovePanel is called, or when we're switching to a new scene.
 *
 * Here's a typical example of using this function:
 *
 * @example @code
 * void drawMyPanel(Panel* panel, PastequeGameState* gameState, void* data) {
 *     panelDrawText(panel, 4, 6, "Hello world!", PASTEQUE_COLOR_WHITE);
 * }
 *
 * void init(PastequeGameState* gameState) {
 *     gsAddPanel(gameState, 0, 0, 20, 20, noneAdornment, &drawMyPanel, NULL);
 * }
 * @endcode
 *
 * @param pGameState the game state
 * @param x the X coordinate of the panel
 * @param y the Y coordinate of the panel
 * @param width the width of the panel
 * @param height the height of the panel
 * @param adornment an optional adornment drawn outside the panel (use noneAdornment to disable it)
 * @param drawFunc the draw function called when drawing the panel
 * @param pPanelData an optional pointer to fuel some data into the draw function
 * @return the panel, owned by the game state
 */
Panel* gsAddPanel(PastequeGameState* pGameState, int x, int y, int width, int height,
                  PanelAdornment adornment, DrawPanelFunction drawFunc, void* pPanelData);

/**
 * Moves the given panel to another layer. Panels on higher layers render after other panels.
 * @param pGameState the game state
 * @param pPanel the panel whose layer will change
 * @param newLayer the new layer of the panel
 */
void gsMovePanelLayer(PastequeGameState* pGameState, Panel* pPanel, int newLayer);

/**
 * Removes a panel from the game state.
 * If a panel has already been removed, removing it twice won't have any effect.
 * @param pGameState the game state
 * @param panel the panel to remove
 */
void gsRemovePanel(PastequeGameState* pGameState, Panel* panel);

/**
 * Removes all present panels from the game state. Usually called when switching scenes.
 * @param pGameState the game state
 */
void gsRemoveAllPanels(PastequeGameState* pGameState);

/**
 * Draws all panels contained in the game state, by calling the draw functions.
 * The drawing order is determined using the layers of the panels.
 * @param pGameState the game state
 */
void gsDrawAllPanels(PastequeGameState* pGameState);

/**
 * Switches from a scene to another. The scene data must be created using a makeXData() function,
 * and must correspond to the given scene name.
 * This function shouldn't be called during draw functions. It is preferable to not run any code
 * after calling this function (use return!).
 *
 * @param pGameState the game state
 * @param newScene the name of the new scene
 * @param newSceneData the scene data associated to that scene
 */
void gsSwitchScene(PastequeGameState* pGameState, SceneName newScene, void* newSceneData);

/**
 * Quits the game on the next frame. Also calls any sceneFinish functions.
 * @param pGameState the game state
 */
void gsQuitGame(PastequeGameState* pGameState);

#endif //PROJET_PASTEQUE_GAME_STATE_H
