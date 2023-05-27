#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "scenes/main_menu_scene.h"
#include "colors.h"
#include "scenes/crush_scene.h"
#include "stdbool.h"
#include "ui.h"
#include "board.h"
#include "highscore.h"
#include "scenes/story_scene.h"

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
char* ArrowLeft = "◀";

struct MainMenuData_S {
    Panel* titlePanel;
    Panel* subtitlePanel;
    Panel* adPanel; // #OPÉ_SPÉ

    // The main buttons (Play, High Scores, Quit)
    struct MainSubMenu {
        UIState state;
        ToggleOption playButton;
        ToggleOption resumeButton;
        ToggleOption storyModeButton;
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

    // The high scores menu, showing all high scores
    // filtered by a preset and symbol count
    struct HighScoreSubMenu {
        UIState state;
        ToggleOption presetOption;
        ToggleOption symbolsOption;
        ToggleOption backButton;

        BoardSizePreset presetFilter;
        char symbolsFilter;

        player allPlayers[MAX_PLAYERS];
        int numAllPlayers;
        // This is an array of pointers pointing to
        // elements of the allPlayers array.
        // Saves up some memory.
        player* filteredPlayers[MAX_PLAYERS];
        int numFilteredPlayers;
    } highScoreUI;
    Panel* highScorePanel;

    NotificationPanelData notificationData;
};

typedef struct MainSubMenu MainSubMenu;
typedef struct PlaySubMenu PlaySubMenu;
typedef struct HighScoreSubMenu HighScoreSubMenu;
typedef struct PlaySettings PlaySettings;

MainMenuData* makeMainMenuData() {
    MainMenuData* data = calloc(1, sizeof(MainMenuData));
    if (data == NULL) {
        RAGE_QUIT(1000, "Failed to allocate MainMenuData.");
    }
    return data;
}

// -------------------------------------------------------
// MAIN UI FUNCTIONS
// Contains: Title, Subtitle, Ad, Play, High Scores, Quit
// --------------------------------------------------------

void drawTitlePanel(Panel* panel, PastequeGameState* gameState, void* osef) {
    for (int i = 0; i < panel->height; ++i) {
        panelDrawLine(panel, 0, i, panel->width, ' ', PASTEQUE_COLOR_WATERMELON_BG_DYN);
    }
    panelDrawText(panel, TITLE_MARGIN_HALF, TITLE_MARGIN_HALF, TitleAsciiArtL1, PASTEQUE_COLOR_WATERMELON_BG_DYN);
    panelDrawText(panel, TITLE_MARGIN_HALF, TITLE_MARGIN_HALF + 1, TitleAsciiArtL2, PASTEQUE_COLOR_WATERMELON_BG_DYN);
    panelDrawText(panel, TITLE_MARGIN_HALF, TITLE_MARGIN_HALF + 2, TitleAsciiArtL3, PASTEQUE_COLOR_WATERMELON_BG_DYN);
    panelDrawText(panel, TITLE_MARGIN_HALF, TITLE_MARGIN_HALF + 3, TitleAsciiArtL4, PASTEQUE_COLOR_WATERMELON_BG_DYN);
}

void drawSubtitlePanel(Panel* panel, PastequeGameState* gameState, void* osef) {
    panelDrawText(panel, 0, 0, "Un jeu de crush vraiment bon !", PASTEQUE_COLOR_WHITE);
}

void drawMainUI(Panel* panel, PastequeGameState* gameState, void* panelData) {
    MainMenuData* data = panelData;
    MainSubMenu* ui = &data->mainUI;

    uiDrawToggleOption(panel, &ui->state, &ui->playButton, 2, 0, BUTTON_WIDTH, "Jouer", 0, toggleStyleButton);
    uiDrawToggleOption(panel, &ui->state, &ui->resumeButton, 2, 2, BUTTON_WIDTH, "Reprendre", 1, toggleStyleButton);
    uiDrawToggleOption(panel, &ui->state, &ui->storyModeButton, 2, 4, BUTTON_WIDTH, "Mode histoire", 2,
                       toggleStyleButton);
    uiDrawToggleOption(panel, &ui->state, &ui->highScoresButton, 2, 6, BUTTON_WIDTH, "Meilleurs scores", 3,
                       toggleStyleButton);
    uiDrawToggleOption(panel, &ui->state, &ui->quitButton, 2, 8, BUTTON_WIDTH, "Quitter", 4, toggleStyleButton);

    // Draw the little arrow on the left.
    if (ui->state.focused) {
        panelDrawText(panel, 0, ui->state.selectedIndex * 2, ArrowRight, PASTEQUE_COLOR_WATERMELON_DYN);
    }
}

void drawAdPanel(Panel* panel, PastequeGameState* gameState, void* osef) {
    panelDrawText(panel, 0, 0, "Participez au Tournoi\ninternational de Pasteque\net tentez de gagner\nle gros lot !\n"
                               "Plus d'informations\nprochainement.",
                  PASTEQUE_COLOR_YELLOW);
}

// -------------------------------------------------------
// PLAY UI FUNCTIONS
// Contains: Symbol Count, Board Size, Play Button
// --------------------------------------------------------

void drawPlayUI(Panel* panel, PastequeGameState* gameState, void* panelData) {
    MainMenuData* data = panelData;
    PlaySubMenu* ui = &data->playUI;

    // Draw the background
    for (int y = 0; y < 3; ++y) {
        panelDrawLine(panel, 0, y, panel->width, ' ', PASTEQUE_COLOR_BLACK);
    }

    panelDrawTextCentered(panel, -1, 1, "Nouvelle partie", PASTEQUE_COLOR_BLACK);
    panelDrawTextCentered(panel, -1, 4, "Nombre de symboles", PASTEQUE_COLOR_WHITE);

    uiDrawToggleOption(panel, &ui->state, &ui->symbolOptions[0], 5, 6, 3, "4", 0, toggleStyleDefault);
    uiDrawToggleOption(panel, &ui->state, &ui->symbolOptions[1], 11, 6, 3, "5", 1, toggleStyleDefault);
    uiDrawToggleOption(panel, &ui->state, &ui->symbolOptions[2], 17, 6, 3, "6", 2, toggleStyleDefault);

    panelDrawTextCentered(panel, -1, 8, "Taille de la grille", PASTEQUE_COLOR_WHITE);
    uiDrawToggleOption(panel, &ui->state, &ui->presetOptions[0], 2, 10, 22, "Petite (10x7)", 3, toggleStyleDefault);
    uiDrawToggleOption(panel, &ui->state, &ui->presetOptions[1], 2, 12, 22, "Moyenne (15x10)", 4, toggleStyleDefault);
    uiDrawToggleOption(panel, &ui->state, &ui->presetOptions[2], 2, 14, 22, "Grande (24x16)", 5, toggleStyleDefault);
    uiDrawToggleOption(panel, &ui->state, &ui->presetOptions[3], 2, 16, 22, "Personnalisée", 6, toggleStyleDefault);

    // Update the text for width/height input boxes.
    if (!ui->widthInput.isWriting) {
        sprintf(ui->widthInput.inputText, "%d", data->playSettings.width);
    }
    if (!ui->heightInput.isWriting) {
        sprintf(ui->heightInput.inputText, "%d", data->playSettings.height);
    }
    uiDrawTextInput(panel, &ui->state, &ui->widthInput, 5, 17, 4, 3, 7, textInputStyleDefault);
    panelDrawLine(panel, 12, 17, 1, 'X', PASTEQUE_COLOR_WHITE);
    uiDrawTextInput(panel, &ui->state, &ui->heightInput, 16, 17, 4, 3, 8, textInputStyleDefault);

    uiDrawToggleOption(panel, &ui->state, &ui->playButton, 1, 19, panel->width - 2, "Démarrer !!", 9,
                       toggleStyleButton);
    uiDrawToggleOption(panel, &ui->state, &ui->backButton, 1, 21, panel->width - 2, "Annuler", 10,
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

// -------------------------------------------------------
// HIGH SCORES UI FUNCTIONS
// Contains: High scores (duh), symbol and preset filters
// --------------------------------------------------------

// Declare some functions beforehand (looks more clean... I think)
char* getPresetLabel(BoardSizePreset preset);

void drawHighScoreUI(Panel* panel, PastequeGameState* gameState, void* panelData) {
    MainMenuData* data = panelData;
    HighScoreSubMenu* ui = &data->highScoreUI;

    // Draw the background
    for (int y = 0; y < 3; ++y) {
        panelDrawLine(panel, 0, y, panel->width, ' ', PASTEQUE_COLOR_BLACK);
    }

    panelDrawTextCentered(panel, -1, 1, "Meilleurs scores", PASTEQUE_COLOR_BLACK);

    char* presetLabel = getPresetLabel(ui->presetFilter);
    char symbolsLabel[32];
    snprintf(symbolsLabel, 32, "%d symboles", ui->symbolsFilter);

    uiDrawToggleOption(panel, &ui->state, &ui->symbolsOption, 0, 4, panel->width,
                       symbolsLabel, 0, toggleStyleDefault);
    uiDrawToggleOption(panel, &ui->state, &ui->presetOption, 0, 6, panel->width,
                       presetLabel, 1, toggleStyleDefault);

    // Draw the arrows
    ColorId symbolColor = uiGetToggleOptionColor(&ui->state, &ui->symbolsOption);
    ColorId presetColor = uiGetToggleOptionColor(&ui->state, &ui->presetOption);
    panelDrawText(panel, 0, 4, ArrowLeft, symbolColor);
    panelDrawText(panel, panel->width - 1, 4, ArrowRight, symbolColor);
    panelDrawText(panel, 0, 6, ArrowLeft, presetColor);
    panelDrawText(panel, panel->width - 1, 6, ArrowRight, presetColor);

    int displayedPlayers = ui->numFilteredPlayers;

    for (int i = 0; i < displayedPlayers; i++) {
        player* pl = ui->filteredPlayers[i];

        panelDrawText(panel, 0, 8 + i, pl->name, PASTEQUE_COLOR_WHITE);
        char scoreStr[16];
        snprintf(scoreStr, 16, "%d", pl->score);
        panelDrawText(panel, panel->width - (int) strlen(scoreStr), 8 + i, scoreStr, PASTEQUE_COLOR_WHITE);
    }

    uiDrawToggleOption(panel, &ui->state, &ui->backButton, 1, 21, panel->width - 2,
                       "Retour", 2, toggleStyleButton);
}

char* getPresetLabel(BoardSizePreset preset) {
    switch (preset) {
        case BSP_SMALL:
            return "Petite grile";
        case BSP_MEDIUM:
            return "Moyenne grille";
        case BSP_LARGE:
            return "Grande grille";
        case BSP_CUSTOM:
            return "Grille perso.";
        default:
            return "???";
    }
}

// Update all the highscores; and the filtered highscores.
void updateHighscores(MainMenuData* data, bool readFile) {
    HighScoreSubMenu* ui = &data->highScoreUI;

    // Read the high scores from the saved file.
    if (readFile) {
        // Reset anything beforehand
        memset(ui->allPlayers, 0, sizeof(player) * MAX_PLAYERS);
        ui->numAllPlayers = 0;

        // Make sure the file exists (https://stackoverflow.com/a/230068)
        if (access("highscore.pasteque", F_OK) == 0) {
            if (!hsParse("highscore.pasteque", ui->allPlayers, &ui->numAllPlayers)) {
                uiDisplayNotification(&data->notificationData,
                                      "Erreur lors de la lecture du fichier des meilleurs scores.",
                                      PASTEQUE_COLOR_WHITE_ON_RED, MICROS(5000));
            }
        } else {
            uiDisplayNotification(&data->notificationData, "Le fichier contenant les meilleurs scores n'existe pas.",
                                  PASTEQUE_COLOR_WHITE_ON_DARK_ORANGE, MICROS(5000));
        }
    }

    memset(ui->filteredPlayers, 0, sizeof(player*) * MAX_PLAYERS);
    ui->numFilteredPlayers = 0;

    int filteredIdx = 0;
    for (int i = 0; i < ui->numAllPlayers; i++) {
        player* pl = &ui->allPlayers[i];
        if (pl->preset == ui->presetFilter &&
            pl->symbols == ui->symbolsFilter) {
            ui->filteredPlayers[filteredIdx] = pl;
            filteredIdx++;
        }
    }
    ui->numFilteredPlayers = filteredIdx;
}

// Direction must be either 1 or -1
void moveHighscoreFilter(MainMenuData* data, int direction) {
    HighScoreSubMenu* ui = &data->highScoreUI;

    if (ui->state.selectedIndex == ui->symbolsOption.interactionIndex) {
        // Loop back to previous options.
        if (direction == -1 && ui->symbolsFilter == 4) {
            ui->symbolsFilter = 6;
        } else {
            ui->symbolsFilter = 4 + ((ui->symbolsFilter - 4) + direction) % 3;
        }
        updateHighscores(data, false);
    } else if (ui->state.selectedIndex == ui->presetOption.interactionIndex) {
        if (direction == -1 && ui->presetFilter == 0) {
            ui->presetFilter = 3;
        } else {
            ui->presetFilter = (ui->presetFilter + direction) % 4;
        }
        updateHighscores(data, false);
    }
}

// -----------------------------------------------
// OTHER UI FUNCTIONS
// -----------------------------------------------

void switchSubMenu(MainMenuData* data, Panel* panel, UIState* state) {
    // Switch off all other submenus (except the mainUI panel)
    data->playUI.state.focused = false;
    data->playPanel->visible = false;
    data->highScoreUI.state.focused = false;
    data->highScorePanel->visible = false;
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
    adorn.colorPairOverrideV = PASTEQUE_COLOR_WHITE_ON_WATERMELON_DYN;
    adorn.colorPairOverrideEndY = 100;

    data->titlePanel = gsAddPanel(gameState, 2, 2, TITLE_WIDTH + TITLE_MARGIN, TITLE_HEIGHT + TITLE_MARGIN, adorn,
                                  &drawTitlePanel, NULL);
    data->subtitlePanel = gsAddPanel(gameState, 2, 11, 30, 2, noneAdornment, &drawSubtitlePanel, NULL);

    data->mainUIPanel = gsAddPanel(gameState, 4, 14, 26, 10, noneAdornment, &drawMainUI, data);
    data->mainUI.state.focused = true;

    data->adPanel = gsAddPanel(gameState, 4, 25, 26, 6, makeAdornment(PAS_SINGLE_BORDER, PASTEQUE_COLOR_YELLOW),
                               &drawAdPanel, NULL);

    // PLAY SUBMENU (Symbols, dimensions)
    // --------------------------------------------------------
    PanelAdornment sideAdorn = makeAdornment(PAS_CLOSE_BORDER, PASTEQUE_COLOR_WHITE);
    sideAdorn.colorPairOverrideV = PASTEQUE_COLOR_WHITE_ON_WHITE;
    sideAdorn.colorPairOverrideStartY = 0;
    sideAdorn.colorPairOverrideEndY = 2;
    data->playPanel = gsAddPanel(gameState, TITLE_WIDTH + TITLE_MARGIN + 5, 2, 26, 23,
                                 sideAdorn, &drawPlayUI, data);

    data->playPanel->visible = false;
    data->playUI.state.focused = false;
    data->playUI.symbolOptions[0].toggled = true;
    data->playUI.presetOptions[1].toggled = true;

    // HIGH SCORES SUBMENU
    // --------------------------------------------------------
    data->highScorePanel = gsAddPanel(gameState, TITLE_WIDTH + TITLE_MARGIN + 5, 2, 32, 23,
                                      sideAdorn, &drawHighScoreUI, data);

    data->highScorePanel->visible = false;
    data->highScoreUI.state.focused = false;
    data->highScoreUI.presetFilter = BSP_MEDIUM;
    data->highScoreUI.symbolsFilter = 4;

    // NOTIFICATION PANEL
    // --------------------------------------------------------
    uiAddNotificationPanel(gameState, 50, &data->notificationData);

    // PLAY SETTINGS CONFIG
    // --------------------------------------------------------
    data->playSettings.sizePreset = BSP_MEDIUM;
    boardGetPresetDimensions(BSP_MEDIUM, &data->playSettings.width, &data->playSettings.height);
    data->playSettings.symbols = 4;
}

void mainMenuUpdate(PastequeGameState* gameState, MainMenuData* data, unsigned long deltaTime) {
    uiUpdateNotificationPanel(&data->notificationData, deltaTime);
}

void mainMenuEvent(PastequeGameState* gameState, MainMenuData* data, Event* pEvent) {
    MainSubMenu* mainUI = &data->mainUI;
    PlaySubMenu* playUI = &data->playUI;
    HighScoreSubMenu* hsUI = &data->highScoreUI;

    // We're in the main UI buttons
    if (mainUI->state.focused) {
        // Add keyboard navigation (down and up arrows keys, Z and S keys)
        UINavBlock blocks[] = {{0, 4, ND_VERTICAL}};
        uiKeyboardNav(&mainUI->state, pEvent, blocks, 1);

        // Handle any button click.
        if (uiHandleToggleOptionEvent(&mainUI->state, &mainUI->playButton, pEvent)) {
            switchSubMenu(data, data->playPanel, &data->playUI.state);
            return;
        } else if (uiHandleToggleOptionEvent(&mainUI->state, &mainUI->resumeButton, pEvent)) {
            char errMsg[256];
            CrushBoard* board;
            // Make sure the file exists.
            if (access("savefile.pasteque", F_OK) == 0) {
                if (boardReadFromFile("savefile.pasteque", &board, errMsg)) {
                    gsSwitchScene(gameState, SN_CRUSH, makeCrushData(board, CIM_ALL));
                    return;
                } else {
                    uiDisplayNotification(&data->notificationData, errMsg, PASTEQUE_COLOR_WHITE_ON_RED, MICROS(5000));
                }
            } else {
                uiDisplayNotification(&data->notificationData, "Aucune partie n'a été sauvegardée.",
                                      PASTEQUE_COLOR_WHITE_ON_DARK_ORANGE, MICROS(5000));
            }
        } else if (uiHandleToggleOptionEvent(&mainUI->state, &mainUI->storyModeButton, pEvent)) {
            gsSwitchScene(gameState, SN_STORY, makeStoryData(0));
        } else if (uiHandleToggleOptionEvent(&mainUI->state, &mainUI->highScoresButton, pEvent)) {
            updateHighscores(data, true);
            switchSubMenu(data, data->highScorePanel, &data->highScoreUI.state);
        } else if (uiHandleToggleOptionEvent(&mainUI->state, &mainUI->quitButton, pEvent)) {
            gsQuitGame(gameState);
        }
    } else if (playUI->state.focused) { // In the play side panel
        // Text input events should be first so they get the maximum priority.
        if (uiHandleTextInputEvent(&playUI->state, &playUI->widthInput, pEvent)) {
            if (!playUI->widthInput.isWriting) {
                // We're done writing.
                registerWidthInput(data, &playUI->widthInput);
            }
            return;
        }
        if (uiHandleTextInputEvent(&playUI->state, &playUI->heightInput, pEvent)) {
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
            if (uiHandleToggleOptionEvent(&playUI->state, option, pEvent)) {
                selectSymbolOption(data, option);
                return;
            }
        }

        for (int i = 0; i < 4; ++i) {
            ToggleOption* option = &playUI->presetOptions[i];
            if (uiHandleToggleOptionEvent(&playUI->state, option, pEvent)) {
                selectPresetOption(data, option);
                return;
            }
        }

        if (uiHandleToggleOptionEvent(&playUI->state, &playUI->playButton, pEvent)) {
            PlaySettings params = data->playSettings;
            CrushBoard* board = makeCrushBoard(params.sizePreset, params.width, params.height, params.symbols);
            gsSwitchScene(gameState, SN_CRUSH, makeCrushData(board, CIM_ALL));
            return;
        }

        if (uiHandleToggleOptionEvent(&playUI->state, &playUI->backButton, pEvent)) {
            switchSubMenu(data, data->mainUIPanel, &mainUI->state);
        }
    } else if (hsUI->state.focused) {
        if (uiHandleToggleOptionEvent(&hsUI->state, &hsUI->symbolsOption, pEvent)) {
            moveHighscoreFilter(data, 1);
            return;
        } else if (uiHandleToggleOptionEvent(&hsUI->state, &hsUI->presetOption, pEvent)) {
            moveHighscoreFilter(data, 1);
            return;
        }
        if (uiHandleToggleOptionEvent(&hsUI->state, &hsUI->backButton, pEvent)) {
            switchSubMenu(data, data->mainUIPanel, &mainUI->state);
            return;
        }

        UINavBlock blocks[] = {
                {0, 2, ND_VERTICAL}
        };
        uiKeyboardNav(&hsUI->state, pEvent, blocks, 1);

        if (hsUI->state.selectedIndex == hsUI->symbolsOption.interactionIndex
            || hsUI->state.selectedIndex == hsUI->presetOption.interactionIndex) {
            if (pEvent->code == KEY_LEFT || pEvent->code == KEY_Q) {
                moveHighscoreFilter(data, -1);
            } else if (pEvent->code == KEY_RIGHT || pEvent->code == KEY_D) {
                moveHighscoreFilter(data, 1);
            }
        }
    }
}

void mainMenuFinish(PastequeGameState* gameState, MainMenuData* data) {
}
