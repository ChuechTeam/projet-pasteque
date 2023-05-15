#include <malloc.h>
#include "scenes/main_menu_scene.h"
#include "colors.h"
#include "scenes/crush_scene.h"
#include "stdbool.h"
#include "ui.h"

#define TITLE_WIDTH 40
#define TITLE_HEIGHT 4
#define TITLE_MARGIN 3
#define TITLE_MARGIN_HALF (TITLE_MARGIN/2)
#define BUTTON_WIDTH 22

char* TitleAsciiArtL1 = "  ___  _   ___ _____ ___ ___  _   _ ___ ";
char* TitleAsciiArtL2 = " | _ \\/_\\ / __|_   _| __/ _ \\| | | | __|";
char* TitleAsciiArtL3 = " |  _/ _ \\\\__ \\ | | | _| (_) | |_| | _| ";
char* TitleAsciiArtL4 = " |_|/_/ \\_\\___/ |_| |___\\__\\_\\\\___/|___|";
char* ArrowRight = "▶";

struct MainMenuData_S {
    Panel* titlePanel;
    Panel* subtitlePanel;

    // The main buttons (Play, High Scores, Quit)
    struct MainSubMenu {
        UIState state;
        ToggleOption playButton;
        ToggleOption highScoresButton;
        ToggleOption quitButton;
    } mainUI;
    Panel* mainUIPanel;

    // The side panel with options for configuring the game
    struct PlaySubMenu {
        UIState state;
        ToggleOption symbolOptions[3];
        TextInput columnsInput;
        TextInput rowsInput;
        ToggleOption playButton;
        ToggleOption backButton;
    } playUI;
    Panel* playPanel;

    struct PlaySettings {
        char symbols;
        int rows;
        int columns;
    } playSettings;
};

typedef struct MainSubMenu MainSubMenu;
typedef struct PlaySubMenu PlaySubMenu;
typedef struct PlaySettings PlaySettings;

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

void drawMainUI(Panel* panel, PastequeGameState* gameState, void* panelData) {
    MainMenuData* data = panelData;
    MainSubMenu* ui = &data->mainUI;

    drawToggleOption(panel, &ui->state, &ui->playButton, 2, 0, BUTTON_WIDTH, "Jouer", 0, toggleStyleButton);
    drawToggleOption(panel, &ui->state, &ui->highScoresButton, 2, 2, BUTTON_WIDTH, "Meilleurs scores", 1, toggleStyleButton);
    drawToggleOption(panel, &ui->state, &ui->quitButton, 2, 4, BUTTON_WIDTH, "Quitter", 2, toggleStyleButton);

    // Draw the little arrow on the left.
    if (ui->state.focused) {
        panelDrawText(panel, 0, ui->state.selectedIndex * 2, ArrowRight, PASTEQUE_COLOR_WHITE);
    }
}

void drawSidePanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    MainMenuData* data = panelData;
    PlaySubMenu* ui = &data->playUI;

    // Draw the background
    for (int y = 0; y < 3; ++y) {
        panelDrawLine(panel, 0, y, panel->width, ' ', PASTEQUE_COLOR_BLACK);
    }
    panelDrawTextCentered(panel, -1, 1, "Sous-menu", PASTEQUE_COLOR_BLACK);
    panelDrawTextCentered(panel, -1, 5, "Nombre de symboles", PASTEQUE_COLOR_WHITE);

    drawToggleOption(panel, &ui->state, &ui->symbolOptions[0], 5, 7, 3, "4", 0, toggleStyleDefault);
    drawToggleOption(panel, &ui->state, &ui->symbolOptions[1], 11, 7, 3, "5", 1, toggleStyleDefault);
    drawToggleOption(panel, &ui->state, &ui->symbolOptions[2], 17, 7, 3, "6", 2, toggleStyleDefault);

    if (!ui->columnsInput.isWriting) {
        sprintf(ui->columnsInput.inputText, "%d", data->playSettings.columns);
    }
    drawTextInput(panel, &ui->state, &ui->columnsInput, 5, 9, 4, 3, 3, textInputStyleDefault);

    drawToggleOption(panel, &ui->state, &ui->playButton, 1, 11, panel->width - 2, "Démarrer !!", 4, toggleStyleButton);
}

void selectSymbolOption(MainMenuData* data, ToggleOption* selected) {
    for (int i = 0; i < 3; ++i) {
        ToggleOption* option = &data->playUI.symbolOptions[i];
        if (option == selected) {
            data->playSettings.symbols = i + 4;
            option->toggled = true;
        } else {
            option->toggled = false;
        }
    }
}

// -----------------------------------------------
// GAME LIFECYCLE FUNCTIONS
// -----------------------------------------------

void mainMenuInit(PastequeGameState* gameState, MainMenuData* data) {
    PanelAdornment adorn = makeAdornment(PAS_CLOSE_BORDER, PASTEQUE_COLOR_WHITE);
    data->titlePanel = gsAddPanel(gameState, 2, 2, TITLE_WIDTH + TITLE_MARGIN, TITLE_HEIGHT + TITLE_MARGIN, adorn,
                                  &drawTitlePanel, NULL);
    data->subtitlePanel = gsAddPanel(gameState, 2, 11, 30, 2, noneAdornment, &drawSubtitlePanel, NULL);

    data->mainUIPanel = gsAddPanel(gameState, 4, 14, 26, 10, noneAdornment, &drawMainUI, data);
    data->mainUI.state.focused = false; // temp!

    PanelAdornment sideAdorn = makeAdornment(PAS_CLOSE_BORDER, PASTEQUE_COLOR_BLUE);
    sideAdorn.colorPairOverrideV = PASTEQUE_COLOR_BLUE_ON_WHITE;
    sideAdorn.colorPairOverrideStartY = 0;
    sideAdorn.colorPairOverrideEndY = 2;
    data->playPanel = gsAddPanel(gameState, TITLE_WIDTH + TITLE_MARGIN + 5, 2, 26, 18,
                                 sideAdorn, &drawSidePanel, data);

    data->playUI.state.focused = true;
    data->playUI.symbolOptions[0].toggled = true;

    data->playSettings.symbols = 4;
    data->playSettings.columns = 12;
    data->playSettings.rows = 8;
}

void mainMenuUpdate(PastequeGameState* gameState, MainMenuData* data, unsigned long deltaTime) {
}

void mainMenuEvent(PastequeGameState* gameState, MainMenuData* data, Event* pEvent) {
    MainSubMenu* mainUI = &data->mainUI;
    PlaySubMenu* playUI = &data->playUI;

    // We're in the main UI buttons
    if (mainUI->state.focused) {
        // Move the selected button when pressing arrow keys or ZQSD.
        if ((pEvent->code == KEY_DOWN || pEvent->code == KEY_S) && mainUI->state.selectedIndex < 2) {
            mainUI->state.selectedIndex++;
        } else if ((pEvent->code == KEY_UP || pEvent->code == KEY_Z) && mainUI->state.selectedIndex > 0) {
            mainUI->state.selectedIndex--;
        }

        // Handle any button click.
        if (handleToggleOptionEvent(&mainUI->state, &mainUI->playButton, pEvent)) {
            gsSwitchScene(gameState, SN_CRUSH, makeCrushData(12, 8, 4, CIM_ALL));
            return;
        } else if (handleToggleOptionEvent(&mainUI->state, &mainUI->highScoresButton, pEvent)) {
            // TODO: High scores maybe??
        } else if (handleToggleOptionEvent(&mainUI->state, &mainUI->quitButton, pEvent)) {
            gsQuitGame(gameState);
        }
    } else if (playUI->state.focused) { // In the play side panel
        // When in the symbols, use left and right to navigate through them.
        // Bottom arrow key should be used to go in the options block below.
        if (handleTextInputEvent(&playUI->state, &playUI->columnsInput, pEvent)) {
            // Handled!
        }
        else if (playUI->state.selectedIndex >= 0 && playUI->state.selectedIndex <= 2) {
            if ((pEvent->code == KEY_LEFT || pEvent->code == KEY_Q) && playUI->state.selectedIndex > 0) {
                playUI->state.selectedIndex--;
            } else if ((pEvent->code == KEY_RIGHT || pEvent->code == KEY_D) && playUI->state.selectedIndex < 2) {
                playUI->state.selectedIndex++;
            }
            if (pEvent->code == KEY_DOWN || pEvent->code == KEY_S) {
                playUI->state.selectedIndex = 3; // The first control below.
            }
        } else {
            if (pEvent->code == KEY_UP || pEvent->code == KEY_Z) {
                playUI->state.selectedIndex--;
            } else if ((pEvent->code == KEY_DOWN || pEvent->code == KEY_S) && playUI->state.selectedIndex < 4) {
                playUI->state.selectedIndex++;
            }
        }

        for (int i = 0; i < 3; ++i) {
            ToggleOption* option = &playUI->symbolOptions[i];
            if (handleToggleOptionEvent(&playUI->state, option, pEvent)) {
                selectSymbolOption(data, option);
            }
        }
        
        if (handleToggleOptionEvent(&playUI->state, &playUI->playButton, pEvent)) {
            struct PlaySettings params = data->playSettings;
            gsSwitchScene(gameState, SN_CRUSH, makeCrushData(params.columns, params.rows, params.symbols, CIM_ALL));
        }
    }
}

void mainMenuDrawBackground(PastequeGameState* gameState, MainMenuData* data, Screen* pScreen) {
}

void mainMenuDrawForeground(PastequeGameState* gameState, MainMenuData* data, Screen* pScreen) {
}

void mainMenuFinish(PastequeGameState* gameState, MainMenuData* data) {
}
