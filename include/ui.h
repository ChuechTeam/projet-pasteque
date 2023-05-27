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

static ToggleOptionStyle toggleStyleDefault = {PASTEQUE_COLOR_WHITE,
                                               PASTEQUE_COLOR_WATERMELON_BG_DYN,
                                               PASTEQUE_COLOR_BLACK,
                                               PASTEQUE_COLOR_WATERMELON_BG_LIGHT_DYN};

static ToggleOptionStyle toggleStyleButton = {PASTEQUE_COLOR_WHITE,
                                              PASTEQUE_COLOR_WATERMELON_BG_DYN,
                                              PASTEQUE_COLOR_WATERMELON_BG_DYN,
                                              PASTEQUE_COLOR_WATERMELON_BG_DYN};

static TextInputStyle textInputStyleDefault = {PASTEQUE_COLOR_BLACK,
                                               PASTEQUE_COLOR_WATERMELON_BG_DYN,
                                               PASTEQUE_COLOR_WATERMELON_BG_LIGHT_DYN};

ColorId uiGetToggleOptionColor(const UIState* state, const ToggleOption* option);

void uiDrawToggleOption(Panel* panel, UIState* state, ToggleOption* option, int x, int y, int width, char* text,
                        int interactionIndex, ToggleOptionStyle style);
// True when toggled
bool uiHandleToggleOptionEvent(UIState* state, ToggleOption* option, Event* event);

void uiDrawTextInput(Panel* panel, UIState* state, TextInput* input, int x, int y, int width, int maxLength,
                     int interactionIndex,
                     TextInputStyle style);

bool uiHandleTextInputEvent(UIState* state, TextInput* input, Event* event);

// End is inclusive!
void uiKeyboardNav(UIState* state, Event* event, UINavBlock blocks[], int nBlocks);

// ----
// NOTIFICATION PANEL
// ----

Panel* uiAddNotificationPanel(struct PastequeGameState_S* gameState, int maxWidth, NotificationPanelData* data);

void uiDisplayNotification(NotificationPanelData* data, char* text, ColorId color, unsigned long duration);

void uiUpdateNotificationPanel(NotificationPanelData* data, unsigned long deltaMicros);

#endif //PROJET_PASTEQUE_UI_H
