#include <malloc.h>
#include <stdlib.h>
#include "scenes/main_menu_scene.h"
#include "colors.h"
#include "scenes/crush_scene.h"
#include "stdbool.h"
#include "ui.h"
#include "board.h"

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
        ToggleOption presetOptions[4];
        TextInput widthInput;
        TextInput heightInput;
        ToggleOption playButton;
        ToggleOption backButton;
    } playUI;
    Panel* playPanel;

    struct PlaySettings {
        char symbols;
        BoardSizePreset sizePreset;
        int height;
        int width;
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
    drawToggleOption(panel, &ui->state, &ui->highScoresButton, 2, 2, BUTTON_WIDTH, "Meilleurs scores", 1,
                     toggleStyleButton);
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
    panelDrawTextCentered(panel, -1, 1, "Nouvelle partie", PASTEQUE_COLOR_BLACK);
    panelDrawTextCentered(panel, -1, 4, "Nombre de symboles", PASTEQUE_COLOR_WHITE);

    drawToggleOption(panel, &ui->state, &ui->symbolOptions[0], 5, 6, 3, "4", 0, toggleStyleDefault);
    drawToggleOption(panel, &ui->state, &ui->symbolOptions[1], 11, 6, 3, "5", 1, toggleStyleDefault);
    drawToggleOption(panel, &ui->state, &ui->symbolOptions[2], 17, 6, 3, "6", 2, toggleStyleDefault);

    panelDrawTextCentered(panel, -1, 8, "Taille de la grille", PASTEQUE_COLOR_WHITE);
    drawToggleOption(panel, &ui->state, &ui->presetOptions[0], 2, 10, 22, "Petite (10x7)", 3, toggleStyleDefault);
    drawToggleOption(panel, &ui->state, &ui->presetOptions[1], 2, 12, 22, "Moyenne (15x10)", 4, toggleStyleDefault);
    drawToggleOption(panel, &ui->state, &ui->presetOptions[2], 2, 14, 22, "Grande (24x16)", 5, toggleStyleDefault);
    drawToggleOption(panel, &ui->state, &ui->presetOptions[3], 2, 16, 22, "Personnalisée", 6, toggleStyleDefault);

    // Update the text for width/height input boxes.
    if (!ui->widthInput.isWriting) {
        sprintf(ui->widthInput.inputText, "%d", data->playSettings.width);
    }
    if (!ui->heightInput.isWriting) {
        sprintf(ui->heightInput.inputText, "%d", data->playSettings.height);
    }
    drawTextInput(panel, &ui->state, &ui->widthInput, 5, 17, 4, 3, 7, textInputStyleDefault);
    panelDrawLine(panel, 12, 17, 1, 'X', PASTEQUE_COLOR_WHITE);
    drawTextInput(panel, &ui->state, &ui->heightInput, 16, 17, 4, 3, 8, textInputStyleDefault);

    drawToggleOption(panel, &ui->state, &ui->playButton, 1, 19, panel->width - 2, "Démarrer !!", 9, toggleStyleButton);
    drawToggleOption(panel, &ui->state, &ui->backButton, 1, 21, panel->width - 2, "Annuler", 10,
                     toggleStyleButton);
}

void selectSymbolOption(MainMenuData* data, ToggleOption* selected) {
    for (char i = 0; i < 3; ++i) {
        ToggleOption* option = &data->playUI.symbolOptions[i];
        if (option == selected) {
            data->playSettings.symbols = (char) (i + 4);
            option->toggled = true;
        } else {
            option->toggled = false;
        }
    }
}

void selectPresetOption(MainMenuData* data, ToggleOption* selected) {
    for (int i = 0; i < 4; ++i) {
        ToggleOption* option = &data->playUI.presetOptions[i];
        if (option == selected) {
            BoardSizePreset preset = (BoardSizePreset) i;
            data->playSettings.sizePreset = preset;
            if (preset != BSP_CUSTOM) {
                boardGetPresetDimensions(preset, &data->playSettings.width, &data->playSettings.height);
            }
            option->toggled = true;
        } else {
            option->toggled = false;
        }
    }
}

void registerWidthInput(MainMenuData* data, TextInput* input) {
    // Convert to a number
    char* parseResult;
    long width = strtol(input->inputText, &parseResult, 10);

    // Make sure parsing is successful.
    if (parseResult != input->inputText && data->playSettings.width != width) {
        // Clamp the width
        if (width < 3) { width = 3; }
        else if (width > BOARD_WIDTH_MAX) { width = BOARD_WIDTH_MAX; }

        data->playSettings.width = width;

        // Make sure we have CUSTOM preset
        selectPresetOption(data, &data->playUI.presetOptions[3]);
    }
}

// Same for height
void registerHeightInput(MainMenuData* data, TextInput* input) {
    // Convert to a number
    char* parseResult;
    long height = strtol(input->inputText, &parseResult, 10);

    // Make sure parsing is successful.
    if (parseResult != input->inputText && data->playSettings.height != height) {
        // Clamp the width
        if (height < 3) { height = 3; }
        else if (height > BOARD_HEIGHT_MAX) { height = BOARD_HEIGHT_MAX; }

        data->playSettings.height = height;

        // Make sure we have CUSTOM preset
        selectPresetOption(data, &data->playUI.presetOptions[3]);
    }
}

void switchSubMenu(MainMenuData* data, Panel* panel, UIState* state) {
    // Switch off all other submenus (except the mainUI panel)
    data->playUI.state.focused = false;
    data->playPanel->visible = false;
    data->mainUI.state.focused = false;

    // Switch on the specified submenu.
    panel->visible = true;
    state->focused = true;
    state->selectedIndex = 0; // Reset the index to the default one.
}

// -----------------------------------------------
// GAME LIFECYCLE FUNCTIONS
// -----------------------------------------------

void mainMenuInit(PastequeGameState* gameState, MainMenuData* data) {
    // MAIN UI (Title, Subtitle, Play, High Scores, Quit)
    // --------------------------------------------------------
    PanelAdornment adorn = makeAdornment(PAS_CLOSE_BORDER, PASTEQUE_COLOR_WHITE);
    data->titlePanel = gsAddPanel(gameState, 2, 2, TITLE_WIDTH + TITLE_MARGIN, TITLE_HEIGHT + TITLE_MARGIN, adorn,
                                  &drawTitlePanel, NULL);
    data->subtitlePanel = gsAddPanel(gameState, 2, 11, 30, 2, noneAdornment, &drawSubtitlePanel, NULL);

    data->mainUIPanel = gsAddPanel(gameState, 4, 14, 26, 10, noneAdornment, &drawMainUI, data);
    data->mainUI.state.focused = true;

    // PLAY SUBMENU (Symbols, dimensions)
    // --------------------------------------------------------
    PanelAdornment sideAdorn = makeAdornment(PAS_CLOSE_BORDER, PASTEQUE_COLOR_BLUE);
    sideAdorn.colorPairOverrideV = PASTEQUE_COLOR_BLUE_ON_WHITE;
    sideAdorn.colorPairOverrideStartY = 0;
    sideAdorn.colorPairOverrideEndY = 2;
    data->playPanel = gsAddPanel(gameState, TITLE_WIDTH + TITLE_MARGIN + 5, 2, 26, 23,
                                 sideAdorn, &drawSidePanel, data);

    data->playPanel->visible = false;
    data->playUI.state.focused = false;
    data->playUI.symbolOptions[0].toggled = true;
    data->playUI.presetOptions[1].toggled = true;

    // PLAY SETTINGS CONFIG
    // --------------------------------------------------------
    data->playSettings.sizePreset = BSP_MEDIUM;
    boardGetPresetDimensions(BSP_MEDIUM, &data->playSettings.width, &data->playSettings.height);
    data->playSettings.symbols = 4;
}

void mainMenuUpdate(PastequeGameState* gameState, MainMenuData* data, unsigned long deltaTime) {
}

void mainMenuEvent(PastequeGameState* gameState, MainMenuData* data, Event* pEvent) {
    MainSubMenu* mainUI = &data->mainUI;
    PlaySubMenu* playUI = &data->playUI;

    // We're in the main UI buttons
    if (mainUI->state.focused) {
        // Add keyboard navigation (down and up arrows keys, Z and S keys)
        UINavBlock blocks[] = {{0, 2, ND_VERTICAL}};
        uiKeyboardNav(&mainUI->state, pEvent, blocks, 1);

        // Handle any button click.
        if (handleToggleOptionEvent(&mainUI->state, &mainUI->playButton, pEvent)) {
            switchSubMenu(data, data->playPanel, &data->playUI.state);
            return;
        } else if (handleToggleOptionEvent(&mainUI->state, &mainUI->highScoresButton, pEvent)) {
            // TODO: High scores maybe??
        } else if (handleToggleOptionEvent(&mainUI->state, &mainUI->quitButton, pEvent)) {
            gsQuitGame(gameState);
        }
    } else if (playUI->state.focused) { // In the play side panel
        // Text input events should be first so they get the maximum priority.
        if (handleTextInputEvent(&playUI->state, &playUI->widthInput, pEvent)) {
            if (!playUI->widthInput.isWriting) {
                // We're done writing.
                registerWidthInput(data, &playUI->widthInput);
            }
            return;
        }
        if (handleTextInputEvent(&playUI->state, &playUI->heightInput, pEvent)) {
            if (!playUI->heightInput.isWriting) {
                // We're done writing.
                registerHeightInput(data, &playUI->heightInput);
            }
            return;
        }

        // Add keyboard navigation for multiple blocks:
        UINavBlock blocks[] = {
                {0, 2,  ND_HORIZONTAL}, // Symbol count (4, 5, 6)
                {3, 6,  ND_VERTICAL}, // Presets (S, M, L, Custom)
                {7, 8,  ND_HORIZONTAL}, // Width and Height input
                {9, 10, ND_VERTICAL} // Play and Back buttons
        };
        uiKeyboardNav(&playUI->state, pEvent, blocks, 4);

        for (int i = 0; i < 3; ++i) {
            ToggleOption* option = &playUI->symbolOptions[i];
            if (handleToggleOptionEvent(&playUI->state, option, pEvent)) {
                selectSymbolOption(data, option);
                return;
            }
        }

        for (int i = 0; i < 4; ++i) {
            ToggleOption* option = &playUI->presetOptions[i];
            if (handleToggleOptionEvent(&playUI->state, option, pEvent)) {
                selectPresetOption(data, option);
                return;
            }
        }

        if (handleToggleOptionEvent(&playUI->state, &playUI->playButton, pEvent)) {
            PlaySettings params = data->playSettings;
            CrushData* crush = makeCrushData(params.sizePreset, params.width, params.height, params.symbols, CIM_ALL);
            gsSwitchScene(gameState, SN_CRUSH, crush);
            return;
        }

        if (handleToggleOptionEvent(&playUI->state, &playUI->backButton, pEvent)) {
            switchSubMenu(data, data->mainUIPanel, &mainUI->state);
        }
    }
}

void mainMenuDrawBackground(PastequeGameState* gameState, MainMenuData* data, Screen* pScreen) {
}

void mainMenuDrawForeground(PastequeGameState* gameState, MainMenuData* data, Screen* pScreen) {
}

void mainMenuFinish(PastequeGameState* gameState, MainMenuData* data) {
}
