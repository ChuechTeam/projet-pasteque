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

typedef struct Panel_S {
    int index;
    int x;
    int y;
    int width;
    int height;
    PanelAdornment adornment;
    DrawPanelFunction drawFunc;
    void* pPanelData;
    Screen* pScreen;
} Panel;

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
