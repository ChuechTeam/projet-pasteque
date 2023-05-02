/*
 * Panel.h
 * --------
 * Allows for organized drawing on screen with drawable regions called "panels".
 */

#ifndef PROJET_PASTEQUE_PANEL_H
#define PROJET_PASTEQUE_PANEL_H

#include "libGameRGR2.h"

/**
 * A character style for adorning panels.
 */
typedef enum {
    PAS_NONE,
    PAS_SINGLE_BORDER,
    PAS_DOUBLE_BORDER
} PanelAdornmentStyle;

/**
 * Contains a style and color for adorning panels.
 * Panel adornments are drawn on the outside of the panel.
 */
typedef struct {
    PanelAdornmentStyle style;
    int colorPair;
} PanelAdornment;

extern const PanelAdornment noneAdornment;

struct Panel_S; // Forward declare Panel
struct PastequeGameState_S; // Same for PastequeGameState
typedef void(*DrawPanelFunction)(struct Panel_S* pPanel, struct PastequeGameState_S* pGameState, void* pPanelData);

/**
 * A rectangular region of the screen to draw on.
 * Panels can be placed anywhere, even offscreen.
 */
typedef struct Panel_S {
    /**
     * The index of the Panel in the GameState.
     */
    int index;
    /**
     * The X coordinate of the Panel. Can be offscreen.
     */
    int x;
    /**
    * The Y coordinate of the Panel. Can be offscreen.
    */
    int y;
    /**
     * The width of the Panel. Must be greater than 0.
     */
    int width;
    /**
     * The height of the Panel. Must be greater than 0.
     */
    int height;
    /**
     * The style and color of the adornment drawn on the outside borner of the Panel.
     */
    PanelAdornment adornment;
    /**
     * The draw function called to start drawing the contents of the Panel
     */
    DrawPanelFunction drawFunc;
    /**
     * An optional data structure that can be used inside the Draw Function.
     * This should point to a malloc-allocated structure and will be owned
     * by the Panel, meaning that you shouldn't call `free` on it.
     */
    void* pPanelData;
    /**
     * The RGR Game screen. Used internally for drawing on the screen.
     */
    Screen* pScreen;
} Panel;

/**
 * An empty panel (with a null drawFunc).
 */
extern const Panel emptyPanel;

/**
 * Used internally to create a Panel with validation.
 */
Panel constructPanel(int index, int x, int y, int width, int height,
                PanelAdornment adornment, DrawPanelFunction drawFunc, void* pPanelData, Screen* pScreen);

bool isEmptyPanel(const Panel* pPanel);

void freePanelData(Panel* pPanel);

void drawPanel(Panel* pPanel, struct PastequeGameState_S* pGameState);

void panelDrawLine(Panel* pPanel, int x, int y, int w, char ch, int clrId);

void panelDrawText(Panel* pPanel, int x, int y, char* pText, int clrId);

void panelTranslate(Panel* pPanel, int x, int y);

#endif //PROJET_PASTEQUE_PANEL_H
