#include <malloc.h>
#include <string.h>
#include "scenes/main_menu_scene.h"
#include "colors.h"
#include "scenes/crush_scene.h"
#include "stdbool.h"

#define TITLE_WIDTH 40
#define TITLE_HEIGHT 4
#define TITLE_MARGIN 3
#define TITLE_MARGIN_HALF (TITLE_MARGIN/2)
#define BUTTONS 3
#define BUTTON_WIDTH 22

char* TitleAsciiArtL1 = "  ___  _   ___ _____ ___ ___  _   _ ___ ";
char* TitleAsciiArtL2 = " | _ \\/_\\ / __|_   _| __/ _ \\| | | | __|\n";
char* TitleAsciiArtL3 = " |  _/ _ \\\\__ \\ | | | _| (_) | |_| | _| \n";
char* TitleAsciiArtL4 = " |_|/_/ \\_\\___/ |_| |___\\__\\_\\\\___/|___|";
char* ArrowRight = "▶";

struct MainMenuData_S {
    Panel* titlePanel;
    Panel* subtitlePanel;
    Panel* playButtonPanel;
    Panel* highScoresButtonPanel;
    Panel* quitButtonPanel;
    int focusedButtonIndex; // 0 to BUTTONS-1 (exclusive)
};

MainMenuData* makeMainMenuData() {
    MainMenuData* data = calloc(1, sizeof(MainMenuData));
    if (data == NULL) {
        RAGE_QUIT(1000, "Failed to allocate MainMenuData.");
    }
    return data;
}

// -----------------------------------------------
// PANEL DRAW FUNCTIONS (and other stuff)
// -----------------------------------------------

void drawTitlePanel(Panel* panel, PastequeGameState* gameState, void* osef) {
    panelDrawText(panel, TITLE_MARGIN_HALF, TITLE_MARGIN_HALF, TitleAsciiArtL1, PASTEQUE_COLOR_WHITE);
    panelDrawText(panel, TITLE_MARGIN_HALF, TITLE_MARGIN_HALF + 1, TitleAsciiArtL2, PASTEQUE_COLOR_WHITE);
    panelDrawText(panel, TITLE_MARGIN_HALF, TITLE_MARGIN_HALF + 2, TitleAsciiArtL3, PASTEQUE_COLOR_WHITE);
    panelDrawText(panel, TITLE_MARGIN_HALF, TITLE_MARGIN_HALF + 3, TitleAsciiArtL4, PASTEQUE_COLOR_WHITE);
}

void drawSubtitlePanel(Panel* panel, PastequeGameState* gameState, void* osef) {
    panelDrawText(panel, 0, 0, "Un jeu de crush vraiment bon !", PASTEQUE_COLOR_WHITE);
}

void drawButtonGeneric(Panel* panel, char* label, int buttonIndex, void* panelData) {
    MainMenuData* data = panelData;
    ColorId color;
    if (data->focusedButtonIndex == buttonIndex) {
        color = PASTEQUE_COLOR_BLACK;
        // Draw the background of the button.
        panelDrawLine(panel, 2, 0, panel->width - 2, ' ', color);
        // Draw the arrow on the left.
        panelDrawText(panel, 0, 0, ArrowRight, PASTEQUE_COLOR_WHITE);
    } else {
        color = PASTEQUE_COLOR_WHITE;
    }
    panelDrawText(panel, 1 + (BUTTON_WIDTH - (int) strlen(label)) / 2, 0, label, color);
}

void drawPlayButtonPanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    drawButtonGeneric(panel, "Jouer", 0, panelData);
}

void drawHighScoresButtonPanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    drawButtonGeneric(panel, "Meilleurs scores", 1, panelData);
}

void drawQuitButtonPanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    drawButtonGeneric(panel, "Quitter", 2, panelData);
}

// -----------------------------------------------
// GAME LIFECYCLE FUNCTIONS
// -----------------------------------------------

void mainMenuInit(PastequeGameState* gameState, MainMenuData* data) {
    PanelAdornment adorn = {.style = PAS_DOUBLE_BORDER, .colorPair = PASTEQUE_COLOR_WHITE};
    data->titlePanel = gsAddPanel(gameState, 2, 2, TITLE_WIDTH + TITLE_MARGIN, TITLE_HEIGHT + TITLE_MARGIN, adorn,
                                  &drawTitlePanel, NULL);
    data->subtitlePanel = gsAddPanel(gameState, 2, 11, 30, 2, noneAdornment, &drawSubtitlePanel, NULL);

    data->playButtonPanel = gsAddPanel(gameState, 4, 14, BUTTON_WIDTH, 1, noneAdornment, &drawPlayButtonPanel, data);
    data->highScoresButtonPanel = gsAddPanel(gameState, 4, 16, BUTTON_WIDTH, 1, noneAdornment,
                                             &drawHighScoresButtonPanel, data);
    data->quitButtonPanel = gsAddPanel(gameState, 4, 18, BUTTON_WIDTH, 1, noneAdornment, &drawQuitButtonPanel, data);

    data->focusedButtonIndex = 0;
}

void mainMenuUpdate(PastequeGameState* gameState, MainMenuData* data, unsigned long deltaTime) {
}

void mainMenuEvent(PastequeGameState* gameState, MainMenuData* data, Event* pEvent) {
    bool mouseClickedOnButton = false;
    if (pEvent->code == KEY_MOUSE) {
        // Left click OR right click
        bool isMouseClick = (pEvent->mouseEvent.bstate & (BUTTON1_RELEASED | BUTTON3_RELEASED)) != 0;
        
        // Highlight the button when the cursor goes over it.
        // Run the button action when the user does a left/right click.
        if (panelContainsMouse(data->playButtonPanel, pEvent)) {
            data->focusedButtonIndex = 0;
            mouseClickedOnButton = isMouseClick;
        }
        else if (panelContainsMouse(data->highScoresButtonPanel, pEvent)) {
            data->focusedButtonIndex = 1;
            mouseClickedOnButton = isMouseClick;
        }
        else if (panelContainsMouse(data->quitButtonPanel, pEvent)) {
            data->focusedButtonIndex = 2;
            mouseClickedOnButton = isMouseClick;
        }
    }
    else if ((pEvent->code == KEY_UP || pEvent->code == KEY_Z) && data->focusedButtonIndex > 0) {
        // Select button on top
        data->focusedButtonIndex--;
    }
    else if ((pEvent->code == KEY_DOWN || pEvent->code == KEY_S) && data->focusedButtonIndex < BUTTONS -1) {
        // Select button on bottom
        data->focusedButtonIndex++;
    }

    if (pEvent->code == KEY_RETURN || mouseClickedOnButton) {
        // Run the button action
        switch (data->focusedButtonIndex) {
            case 0: // Play
                gsSwitchScene(gameState, SN_CRUSH, makeCrushData(12, 8, 4, CIM_ALL));
                break;
            case 1: // High scores
                // TODO!
                break;
            case 2: // Quit
                gsQuitGame(gameState);
        }
    }
}

void mainMenuDrawBackground(PastequeGameState* gameState, MainMenuData* data, Screen* pScreen) {
}

void mainMenuDrawForeground(PastequeGameState* gameState, MainMenuData* data, Screen* pScreen) {
}

void mainMenuFinish(PastequeGameState* gameState, MainMenuData* data) {
}
