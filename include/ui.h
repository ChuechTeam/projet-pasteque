/**
 * ui.h
 * ------
 * Contains various functions to draw common user interface elements,
 * like text boxes, buttons, etc. The goal is to be able to define the UI
 * seamlessly in the drawPanel function, so we get easy to read code.
 * This part should really be implemented as simply as possible: it's easy
 * to mess up UI and create complex garbage.
 */
#ifndef PROJET_PASTEQUE_UI_H
#define PROJET_PASTEQUE_UI_H

#include "panel.h"
#include "colors.h"

#define TEXT_INPUT_MAX 128
#define NOTIF_TEXT_MAX 256

typedef struct {
    int inactiveColorId;
    int activeColorId;
    int toggledInactiveColorId;
    int toggledActiveColorId;
} ToggleOptionStyle;

typedef struct {
    // Set inside drawPanel
    bool initialized;
    Panel* panel;
    int x;
    int y;
    int width;
    int interactionIndex;
    ToggleOptionStyle style;
    // Set outside drawPanel
    bool toggled;
} ToggleOption;

typedef struct {
    int inactiveColorId;
    int activeColorId;
    int writingColorId;
} TextInputStyle;

typedef struct {
    // Set inside drawPanel
    bool initialized;
    Panel* panel;
    int x;
    int y;
    int width;
    int interactionIndex;
    int maxLength;
    TextInputStyle style;
    // Set outside drawPanel
    char inputText[TEXT_INPUT_MAX];
    bool isWriting;
} TextInput;

typedef struct {
    int selectedIndex;
    bool focused;
} UIState;

typedef enum {
    ND_HORIZONTAL,
    ND_VERTICAL
} NavigationDirection;

typedef struct {
    int startIndex;
    int endIndex;
    NavigationDirection direction;
} UINavBlock;

typedef struct {
    Panel* panel;
    char text[NOTIF_TEXT_MAX];
    char textWrapped[NOTIF_TEXT_MAX];
    int textWrappedLines;
    // True when we need to update textWrapped and textWrappedLines.
    bool textDirty;
    ColorId color;
    long durationMicros;
    int maxWidth;
} NotificationPanelData;

static const ToggleOptionStyle toggleStyleDefault = {PASTEQUE_COLOR_WHITE,
                                                     PASTEQUE_COLOR_WATERMELON_BG_DYN,
                                                     PASTEQUE_COLOR_BLACK,
                                                     PASTEQUE_COLOR_WATERMELON_BG_LIGHT_DYN};

static const ToggleOptionStyle toggleStyleButton = {PASTEQUE_COLOR_WHITE,
                                                    PASTEQUE_COLOR_WATERMELON_BG_DYN,
                                                    PASTEQUE_COLOR_WATERMELON_BG_DYN,
                                                    PASTEQUE_COLOR_WATERMELON_BG_DYN};

static const TextInputStyle textInputStyleDefault = {PASTEQUE_COLOR_BLACK,
                                                     PASTEQUE_COLOR_WATERMELON_BG_DYN,
                                                     PASTEQUE_COLOR_WATERMELON_BG_LIGHT_DYN};

// Returns the color for a toggle option, which can change if it is focused, enabled, etc.
ColorId uiGetToggleOptionColor(const UIState* state, const ToggleOption* option);

// Draws a toggle option inside a panel.
// The ToggleOption will be updated using the option pointer.
// The x, y, width, interactionIndex, and style parameters are considered to be static:
// they are only read once and stored permanently in the ToggleOption struct.
void uiDrawToggleOption(Panel* panel, UIState* state, ToggleOption* option, int x, int y, int width, char* text,
                        int interactionIndex, ToggleOptionStyle style);

// Handles input events for a toggle option: mouse hover, mouse click, space and enter.
// Returns true when the toggle option has been toggled (by clicking or pressing space/enter).
bool uiHandleToggleOptionEvent(UIState* state, ToggleOption* option, Event* event);

// Draws a text input box inside a panel.
// The TextInput will be updated using the input pointer.
// The x, y, width, maxLength, interactionIndex, and style parameters are considered to be static:
// they are only read once and stored permanently in the ToggleOption struct.
void uiDrawTextInput(Panel* panel, UIState* state, TextInput* input, int x, int y, int width, int maxLength,
                     int interactionIndex,
                     TextInputStyle style);

// Handles input events for a text input box: mouse hover, mouse click, enter; any key when typing.
// Returns true when the toggle option is in typing mode, or when it is quitting it.
// Use the isWriting bool to know when the user finished their input.
bool uiHandleTextInputEvent(UIState* state, TextInput* input, Event* event);

// Handles keyboard input for navigating through the UI.
// Accepts an array of UI Navigation Blocks, delimiting zones where the
// UI is horizontal, or vertical: left-right or up-down arrow keys.
// The index ranges are inclusive. For example, you may have this array:
//     {0, 2, ND_HORIZONTAL}, {3, 4, ND_VERTICAL}, {5, 8, ND_HORIZONTAL}
void uiKeyboardNav(UIState* state, Event* event, UINavBlock blocks[], int nBlocks);

// ----
// NOTIFICATION PANEL
// ----

// Adds a notification panel to the game.
// maxWidth contains the maximum width of the displayed text, and of the panel.
// data must point to a valid location, where the state of the
// notification panel will be saved.
Panel* uiAddNotificationPanel(struct PastequeGameState_S* gameState, int maxWidth, NotificationPanelData* data);

// Displays a notification for a specified period of time, in microseconds.
// The text can hold at most 256 characters, and can have a particular color.
void uiDisplayNotification(NotificationPanelData* data, char* text, ColorId color, unsigned long duration);

// Updates the notification panel's timer and layout.
// Must be called in the update function.
void uiUpdateNotificationPanel(NotificationPanelData* data, unsigned long deltaMicros);

#endif //PROJET_PASTEQUE_UI_H
