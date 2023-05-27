/**
 * Panel.h
 * --------
 * Allows for organized drawing on screen with drawable regions called "panels".
 */

#ifndef PROJET_PASTEQUE_PANEL_H
#define PROJET_PASTEQUE_PANEL_H

#include "libGameRGR2.h"
#include <stdbool.h>

/**
 * A character style for adorning panels.
 */
typedef enum {
    PAS_NONE,
    PAS_SINGLE_BORDER,
    PAS_DOUBLE_BORDER,
    PAS_CLOSE_BORDER
} PanelAdornmentStyle;

/**
 * Contains a style and color for adorning panels.
 * Panel adornments are drawn on the outside of the panel.
 */
typedef struct {
    PanelAdornmentStyle style;
    int colorPair;
    // This is used to override the color of vertical lines, so we
    // get good-looking headers (example: boxes).
    int colorPairOverrideV;
    int colorPairOverrideStartY;
    int colorPairOverrideEndY;
} PanelAdornment;

extern const PanelAdornment noneAdornment;

struct Panel_S; // Forward declare Panel
struct PastequeGameState_S; // Same for PastequeGameState
typedef void(* DrawPanelFunction)(struct Panel_S* pPanel, struct PastequeGameState_S* pGameState, void* pPanelData);

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
     * The layer of the Panel. Panels on a higher layer render later. Defaults to 0.
     * Must be set using gsMovePanelLayer.
     */
    int layer;
    /**
     * True when the panel should be displayed on screen. Else, it is not displayed
     * and its draw function isn't called.
     */
    bool visible;
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
     * This pointer will not be freed by the panel.
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
 * Use gsAddPanel to create a panel instead (game_state.h).
 */
Panel constructPanel(int index, int x, int y, int width, int height,
                     PanelAdornment adornment, DrawPanelFunction drawFunc, void* pPanelData, Screen* pScreen);

/**
 * Returns true if a Panel is empty, meaning that it has a null drawFunc and won't
 * draw anything on screen. Raises an error if the Panel is null.
 * @param pPanel the panel to check
 * @return true if the Panel is empty, false otherwise
 */
bool isEmptyPanel(const Panel* pPanel);

/**
 * Draws the panel on screen. Used internally by GameState.
 * @param pPanel the panel to draw
 * @param pGameState the game state
 */
void drawPanel(Panel* pPanel, struct PastequeGameState_S* pGameState);

/**
 * Draws a colored line inside the panel.
 * The line starts at point (x, y), composed of the character ch repeated w times.
 * The coordinates are relative to the panel's position.
 * The line will be cut off if it goes outside the panel's region.
 * Can only be used inside a Draw Function.
 *
 * @param pPanel the panel to draw into
 * @param x the X coordinate
 * @param y the Y coordinate
 * @param w the length of the line
 * @param ch the character to be repeated
 * @param clrId the color of the line
 */
void panelDrawLine(Panel* pPanel, int x, int y, int w, char ch, int clrId);

/**
 * Draws a colored string inside the panel, starting at local coordinates (x, y).
 * IMPORTANT: Currently, the string won't be truncated if it goes outside the panel.
 * Can only be used inside a Draw Function.
 *
 * @param pPanel the panel to draw into
 * @param x the X coordinate
 * @param y the Y coordinate
 * @param pText the string to draw
 * @param clrId the color of the string
 */
void panelDrawText(Panel* pPanel, int x, int y, char* pText, int clrId);

/**
 * Same as panelDrawText, but centers the text inside the panel.
 * A value of -1 for any axis (x or y) will find the centered coordinates.
 */
void panelDrawTextCentered(Panel* pPanel, int x, int y, char* pText, int clrId);

/**
 * Wraps the given text, limited to a certain width, to a given buffer.
 * @param pText the text to wrap
 * @param width the max width of the lines
 * @param hasStoryMarkups whether or not the text contains story markups (/X chars)
 * @param outWrappedText the buffer containing the wrapped text
 * @param wrappedTextSize the size of the buffer
 */
void panelWrapText(const char* pText, int width, bool hasStoryMarkups, char outWrappedText[], int wrappedTextSize);

/**
 * Moves a panel to the given coordinates.
 * The coordinates can be outside of the screen, and even negative.
 *
 * @param pPanel the panel to move
 * @param x the X coordinate
 * @param y the Y coordinate
 */
void panelTranslate(Panel* pPanel, int x, int y);

/**
 * Centers the panel on one or more axis of the screen.
 * @param pPanel the panel
 * @param centerX true when the panel should be centered horizontally
 * @param centerY true when the panel should be centered vertically
 */
void panelCenterScreen(Panel* pPanel, bool centerX, bool centerY);

/**
 * Creates a panel adornment with the given style and color.
 * @param style the style
 * @param color the horizontal color
 * @return the panel adornment
 */
PanelAdornment makeAdornment(PanelAdornmentStyle style, int color);

#endif //PROJET_PASTEQUE_PANEL_H
