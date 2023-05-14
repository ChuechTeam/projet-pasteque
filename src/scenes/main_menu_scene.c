#include <malloc.h>
#include <string.h>
#include "scenes/main_menu_scene.h"
#include "colors.h"
#include "scenes/crush_scene.h"
#include "stdbool.h"
#include "ui.h"

#define TITLE_WIDTH 40
#define TITLE_HEIGHT 4
#define TITLE_MARGIN 3
#define TITLE_MARGIN_HALF (TITLE_MARGIN/2)
#define BUTTONS 3
#define BUTTON_WIDTH 22

char* TitleAsciiArtL1 = "  ___  _   ___ _____ ___ ___  _   _ ___ ";
char* TitleAsciiArtL2 = " | _ \\/_\\ / __|_   _| __/ _ \\| | | | __|";
char* TitleAsciiArtL3 = " |  _/ _ \\\\__ \\ | | | _| (_) | |_| | _| ";
char* TitleAsciiArtL4 = " |_|/_/ \\_\\___/ |_| |___\\__\\_\\\\___/|___|";
char* ArrowRight = "▶";

struct MainMenuData_S {
    Panel* titlePanel;
    Panel* subtitlePanel;
    Panel* playButtonPanel;
    Panel* highScoresButtonPanel;
    Panel* quitButtonPanel;
    Panel* sidePanel;
    ToggleOption sideToggleTests[3];
    UIState sideUIState;
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
        panelDrawLine(panel, 2, 0, panel->width - 2, ' ', PASTEQUE_COLOR_BLACK);
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

void drawSidePanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    MainMenuData* data = panelData;

    for (int y = 0; y < 3; ++y) {
        panelDrawLine(panel, 0, y, panel->width, ' ', PASTEQUE_COLOR_BLACK);
    }
    panelDrawTextCentered(panel, -1, 1, "Sous-menu", PASTEQUE_COLOR_BLACK);

    panelDrawTextCentered(panel, -1, 5, "Nombre de symboles", PASTEQUE_COLOR_WHITE);

    data->sideToggleTests[0] = makeToggleOption(panel, 5, 7, 3, "4", 0, toggleStyleMonochrome);
    drawToggleOption(panel, &data->sideToggleTests[0], &data->sideUIState);

    data->sideToggleTests[1] = makeToggleOption(panel, 11, 7, 3, "5", 1, toggleStyleMonochrome);
    drawToggleOption(panel, &data->sideToggleTests[1], &data->sideUIState);

    data->sideToggleTests[2] = makeToggleOption(panel, 17, 7, 3, "6", 2, toggleStyleMonochrome);
    drawToggleOption(panel, &data->sideToggleTests[2], &data->sideUIState);

    panelDrawTextCentered(panel, -1, 12, "Ce sous-menu fait rien", PASTEQUE_COLOR_WHITE);
    panelDrawTextCentered(panel, -1, 13, "pour l'instant.", PASTEQUE_COLOR_WHITE);

    panelDrawTextCentered(panel, -1, 15, "TODO: Le rendre utile", PASTEQUE_COLOR_WHITE);
}

// -----------------------------------------------
// GAME LIFECYCLE FUNCTIONS
// -----------------------------------------------

void mainMenuInit(PastequeGameState* gameState, MainMenuData* data) {
    PanelAdornment adorn = makeAdornment(PAS_CLOSE_BORDER, PASTEQUE_COLOR_WHITE);
    data->titlePanel = gsAddPanel(gameState, 2, 2, TITLE_WIDTH + TITLE_MARGIN, TITLE_HEIGHT + TITLE_MARGIN, adorn,
                                  &drawTitlePanel, NULL);
    data->subtitlePanel = gsAddPanel(gameState, 2, 11, 30, 2, noneAdornment, &drawSubtitlePanel, NULL);

    data->playButtonPanel = gsAddPanel(gameState, 4, 14, BUTTON_WIDTH, 1, noneAdornment, &drawPlayButtonPanel, data);
    data->highScoresButtonPanel = gsAddPanel(gameState, 4, 16, BUTTON_WIDTH, 1, noneAdornment,
                                             &drawHighScoresButtonPanel, data);
    data->quitButtonPanel = gsAddPanel(gameState, 4, 18, BUTTON_WIDTH, 1, noneAdornment, &drawQuitButtonPanel, data);

    PanelAdornment sideAdorn = makeAdornment(PAS_CLOSE_BORDER, PASTEQUE_COLOR_BLUE);
    sideAdorn.colorPairOverrideV = PASTEQUE_COLOR_BLUE_ON_WHITE;
    sideAdorn.colorPairOverrideStartY = 0;
    sideAdorn.colorPairOverrideEndY = 2;
    data->sidePanel = gsAddPanel(gameState, TITLE_WIDTH + TITLE_MARGIN + 5, 2, 26, 18,
                                 sideAdorn, &drawSidePanel, data);
    data->sideUIState.selectedIndex = 0;

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
        } else if (panelContainsMouse(data->highScoresButtonPanel, pEvent)) {
            data->focusedButtonIndex = 1;
            mouseClickedOnButton = isMouseClick;
        } else if (panelContainsMouse(data->quitButtonPanel, pEvent)) {
            data->focusedButtonIndex = 2;
            mouseClickedOnButton = isMouseClick;
        }
    } else if ((pEvent->code == KEY_UP || pEvent->code == KEY_Z) && data->focusedButtonIndex > 0) {
        // Select button on top
        data->focusedButtonIndex--;
    } else if ((pEvent->code == KEY_DOWN || pEvent->code == KEY_S) && data->focusedButtonIndex < BUTTONS - 1) {
        // Select button on bottom
        data->focusedButtonIndex++;
    }

    if (pEvent->code == KEY_RETURN || pEvent->code == KEY_SPACE || mouseClickedOnButton) {
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

    handleToggleOptionEvent(&data->sideUIState, &data->sideToggleTests[0], pEvent);
    handleToggleOptionEvent(&data->sideUIState, &data->sideToggleTests[1], pEvent);
    handleToggleOptionEvent(&data->sideUIState, &data->sideToggleTests[2], pEvent);

}

void mainMenuDrawBackground(PastequeGameState* gameState, MainMenuData* data, Screen* pScreen) {
}

void mainMenuDrawForeground(PastequeGameState* gameState, MainMenuData* data, Screen* pScreen) {
}

void mainMenuFinish(PastequeGameState* gameState, MainMenuData* data) {
}
