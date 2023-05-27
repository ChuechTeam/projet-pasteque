#include "scenes/crush_scene.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>
#include "colors.h"
#include "board.h"
#include "ui.h"
#include "scenes/main_menu_scene.h"
#include "highscore.h"
#include "scenes/story_scene.h"

#define BLINKING_DURATION_MICROS MICROS(800L)
#define GRAVITY_PERIOD_MICROS MICROS(160L)
#define PI           3.14159265358979323846

typedef enum {
    CPS_WAITING_INPUT,
    CPS_EVALUATING_BOARD,
    CPS_BLINKING,
    CPS_GRAVITY,
    CPS_GAME_OVER
} CrushPlayState;

struct CrushData_S {
    // GAME STATE
    CrushBoard* board;
    CrushPlayState playState;
    unsigned long animTimeMicros;
    bool paused;

    // INPUT
    Point cursor;
    // True when the cursor focuses on a cell to swap with another.
    bool cursorSwapping;
    // The cells to highlight while swapping with the cursor
    Point swapNeighbors[4];
    int numSwapNeighbors;
    Point mousePressPos;
    bool mouseClickPending;
    CrushInputMethod inputMethod;

    // STORY
    bool storyModeActivated;
    // When true, will skip the game and show the next chapter
    // after a period of time.
    bool storyAutoSkip;
    long storyAutoSkipTime;

    // PANELS
    Panel* boardPanel;
    Panel* scorePanel;
    Panel* gameOverPanel;
    Panel* comboPanel;

    struct PauseMenu {
        UIState state;
        ToggleOption mainButton;
        ToggleOption secondaryButton;
        ToggleOption quitButton;
    } pauseUI;
    Panel* pausePanel;

    struct HighScoreDialog {
        UIState state;
        TextInput nameInput;
        ToggleOption confirmButton;
        ToggleOption skipButton;
        int rank;
    } highScoreUI;
    Panel* highScorePanel;

    NotificationPanelData notificationData;
};

typedef struct PauseMenu PauseMenu;
typedef struct HighScoreDialog HighScoreDialog;

// Declare some functions before so we can access them.
void toggleHighScoreUI(CrushData* data);

// ----------
// GENERAL GAME FUNCTIONS
// (Create a game, Symbol char, UI switch)
// ----------

CrushData* makeCrushData(CrushBoard* board, CrushInputMethod inputMethod) {
    CrushData* data = calloc(1, sizeof(CrushData));
    if (data == NULL) {
        RAGE_QUIT(2001, "Failed to allocate CrushData.");
    }

    // This will validate width, height, and the symbols.
    data->board = board;
    data->inputMethod = inputMethod;
    data->storyModeActivated = data->board->storyIndex >= 0;
    return data;
}

// Finds the character for a symbol in the board
char* cellToStr(CrushCell cell, ColorId* outColor) {
    // Can be adjusted for a different theme, etc.
    switch (cell.sym) {
        case 0:
            *outColor = PASTEQUE_COLOR_WHITE;
            return " ";
        case 1:
            *outColor = PASTEQUE_COLOR_GREEN;
            return "O";
        case 2:
            *outColor = PASTEQUE_COLOR_BLUE;
            return "&";
        case 3:
            *outColor = PASTEQUE_COLOR_TOMATO;
            return "#";
        case 4:
            *outColor = PASTEQUE_COLOR_YELLOW;
            return "$";
        case 5:
            *outColor = PASTEQUE_COLOR_FUSCHIA;
            return "@";
        case 6:
            *outColor = PASTEQUE_COLOR_TURQUOISE;
            return "X";
        default:
            *outColor = PASTEQUE_COLOR_WHITE;
            return "?";
    }
}

// ----------
// MAIN GAME PANELS
// (Board, Score, Game Over, Combo)
// ----------

void drawBoardPanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    CrushData* data = panelData;
    CrushBoard* board = data->board;
    for (int c = 0; c < board->cellCount; c++) {
        CrushCell cell = board->cells[c];
        Point pos = boardCellIndexToPos(data->board, c);

        ColorId color;
        char* cellStr = cellToStr(cell, &color);

        // Override the colors in some situations
        if (cell.highlightedNeighbor) {
            color = PASTEQUE_COLOR_YELLOW_HIGHLIGHT_BG;
        } else if (cell.markedForDestruction && data->playState == CPS_BLINKING) {
            color = (gameState->gameTime % MICROS(200)) < MICROS(100) ?
                    PASTEQUE_COLOR_RED_BLINK1_BG :
                    PASTEQUE_COLOR_RED_BLINK2_BG;
        } else if (pointsEqual(data->cursor, pos)) {
            if (data->cursorSwapping) {
                color = PASTEQUE_COLOR_YELLOW_FOCUS_BG;
            } else {
                color = PASTEQUE_COLOR_YELLOW_HIGHLIGHT_BG;
            }
        }

        panelDrawText(panel, 2 * pos.x, pos.y, cellStr, color);
    }
}

void drawScorePanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    CrushData* data = panelData;

    char buffer[64];
    sprintf(buffer, "Score : %d", data->board->score);

    panelDrawText(panel, 0, 0, buffer, PASTEQUE_COLOR_YELLOW);
}

void drawGameOverPanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    panelDrawTextCentered(panel, -1, -1, "GAME OVER !", PASTEQUE_COLOR_ORANGE);
}

void drawComboPanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    CrushData* data = panelData;

    if (data->board->combo == 0) {
        return;
    }
    // In percent, because floating point here would be more a nightmare
    // than a pleasure if you see what I mean
    int scoreMult = 100 + data->board->combo * 20;
    int scoreMultUnit = scoreMult / 100;
    int scoreMultDecimal = scoreMult % 100;

    char buffer[64];
    sprintf(buffer, "COMBO X%d (Score x%d.%02d)", data->board->combo, scoreMultUnit, scoreMultDecimal);
    panelDrawText(panel, 0, 0, buffer, PASTEQUE_COLOR_YELLOW);

    for (int i = 0; i < data->board->combo; ++i) {
        panelDrawText(panel, 0, i + 2, data->board->comboTricks[i], PASTEQUE_COLOR_YELLOW);
    }
}

// ----------
// BOARD INPUT
// (Swap cells using arrow keys, ZQSD, or mouse)
// ----------

void moveCursorDelta(CrushBoard* board, Point* cursor, int deltaX, int deltaY) {
    cursor->x = loopBackX(board, cursor->x + deltaX);
    cursor->y = loopBackY(board, cursor->y + deltaY);
}

void toggleCursorSwapping(CrushData* data) {
    data->mouseClickPending = false;

    if (data->cursorSwapping) {
        data->cursorSwapping = false;
        for (int i = 0; i < data->numSwapNeighbors; ++i) {
            CELL_PT(data->board, data->swapNeighbors[i]).highlightedNeighbor = false;
        }
        data->numSwapNeighbors = 0;
    } else {
        data->cursorSwapping = true;
        boardGetNeighbors(data->board, data->cursor, data->swapNeighbors, &data->numSwapNeighbors);
        for (int i = 0; i < data->numSwapNeighbors; ++i) {
            CELL_PT(data->board, data->swapNeighbors[i]).highlightedNeighbor = true;
        }
    }
}

void runCursorMoveAction(CrushData* data, int deltaX, int deltaY) {
    if (!data->cursorSwapping) {
        moveCursorDelta(data->board, &data->cursor, deltaX, deltaY);
    } else {
        // Exit cursor swap (remove highlight bools)
        toggleCursorSwapping(data);

        // Swap cells!
        Point targetCellPos = {.x= data->cursor.x + deltaX, .y=data->cursor.y + deltaY};
        SwapResult result = boardSwapCells(data->board, data->cursor, targetCellPos, false);

        switch (result) {
            case SR_SUCCESS:
                data->playState = CPS_EVALUATING_BOARD;
                break;
            case SR_NO_MATCH:
                uiDisplayNotification(&data->notificationData,
                                      "Impossible d'échanger pour ne former aucune ligne.",
                                      PASTEQUE_COLOR_WHITE_ON_RED, MICROS(3000));
                break;
            case SR_EMPTY_CELLS:
                uiDisplayNotification(&data->notificationData,
                                      "Impossible d'échanger avec une case vide.",
                                      PASTEQUE_COLOR_WHITE_ON_RED, MICROS(3000));
                break;
            case SR_OUT_OF_BOUNDS:
                uiDisplayNotification(&data->notificationData,
                                      "Impossible d'échanger avec une case inexistante",
                                      PASTEQUE_COLOR_WHITE_ON_RED, MICROS(3000));
                break;
        }
    }
}

void registerMouseCursorMove(CrushData* data, int mouseX, int mouseY) {
    // Panel cell pos is given by
    // (x, y) = (panel->x + 2*x, panel->y + y)
    if ((mouseX - data->boardPanel->x) % 2 != 0) {
        // The cursor is in an empty space between cells. Don't do anything.
        return;
    }

    int cellX = (mouseX - data->boardPanel->x) / 2;
    int cellY = (mouseY - data->boardPanel->y);
    if (cellX >= data->board->width || cellY >= data->board->height || cellX < 0 || cellY < 0) {
        // Out of bounds.
        return;
    }

    data->cursor.x = cellX;
    data->cursor.y = cellY;
}

void registerMousePressed(CrushData* data, int mouseX, int mouseY) {
    bool validClick;
    if (data->cursorSwapping) {
        // Any existing cell is "valid"
        int boardX = (mouseX - data->boardPanel->x) / 2;
        int boardY = (mouseY - data->boardPanel->y);
        validClick = boardX >= 0 && boardY >= 0
                     && boardX < data->board->width && boardY < data->board->height
                     && ((mouseX - data->boardPanel->x) % 2) == 0;
    } else {
        // Only the cursor-selected cell is valid. When moving the mouse,
        // the registerMouseCursorMove function moves the cursor using the mouse
        // location.
        validClick = mouseX == data->boardPanel->x + (data->cursor.x * 2)
                     && mouseY == data->boardPanel->y + data->cursor.y;
    }

    if (validClick) {
        data->mousePressPos.x = mouseX;
        data->mousePressPos.y = mouseY;
        if (!data->cursorSwapping) {
            toggleCursorSwapping(data);
        }
    } else {
        data->mousePressPos.x = -1;
        data->mousePressPos.y = -1;
        if (data->cursorSwapping) {
            toggleCursorSwapping(data);
        }
    }
}

void registerMouseReleased(CrushData* data, int mouseX, int mouseY) {
    if (data->cursorSwapping) {
        if ((data->mousePressPos.x != mouseX || data->mousePressPos.y != mouseY) && data->mousePressPos.x != -1) {
            // Cursor released on another cell.
            int relativeX = mouseX - data->mousePressPos.x;
            int relativeY = mouseY - data->mousePressPos.y;

            int deltaX, deltaY;
            if (relativeY == 0 || relativeX == 0) {
                // Straight line, we can easily find the direction!
                if (relativeX < 0) {
                    deltaX = -1;
                    deltaY = 0;
                } else if (relativeX > 0) {
                    deltaX = 1;
                    deltaY = 0;
                } else if (relativeY > 0) {
                    deltaX = 0;
                    deltaY = 1;
                } else { // relativeY < 0
                    deltaX = 0;
                    deltaY = -1;
                }
            } else {
                // Diagonal line, use polar coordinates to find the direction.
                float radians = atan2f((float) relativeY, (float) relativeX);
                // Convert to degrees for easier manipulation
                float degrees;
                if (radians < 0) {
                    degrees = 360 + radians * 180 / (float) PI;
                } else {
                    degrees = radians * 180 / (float) PI;
                }

                if (degrees >= 315 && degrees <= 360 || degrees >= 0 && degrees <= 45) {
                    deltaX = 1;
                    deltaY = 0;
                } else if (degrees >= 45 && degrees <= 135) {
                    deltaX = 0;
                    deltaY = 1;
                } else if (degrees >= 135 && degrees <= 225) {
                    deltaX = -1;
                    deltaY = 0;
                } else if (degrees >= 225 && degrees < 315) {
                    deltaX = 0;
                    deltaY = -1;
                } else {
                    debug("Failed to find the right delta! [rad=%f; deg=%f]\n", radians, degrees);
                    deltaX = 0;
                    deltaY = 0;
                }
            }

            if (deltaY != 0 || deltaX != 0) {
                runCursorMoveAction(data, deltaX, deltaY);
            }
        } else {
            // Cursor probably released on the same cell.
            if (data->mousePressPos.x != -1) { // Make sure it's valid
                if (data->mouseClickPending) {
                    // Check for the case where we're clicking on a neighbor
                    // Typical situation: clicking on a cell, then clicking on the other cell
                    for (int i = 0; i < data->numSwapNeighbors; ++i) {
                        Point neighbor = data->swapNeighbors[i];
                        int mx = neighbor.x * 2 + data->boardPanel->x;
                        int my = neighbor.y + data->boardPanel->y;
                        if (mx == mouseX && my == mouseY) {
                            // The user clicked on a cell! Find the difference and we're done.
                            runCursorMoveAction(data, neighbor.x - data->cursor.x, neighbor.y - data->cursor.y);
                        }
                    }
                    // If that didn't work, cancel the action.
                    // Typical situation: clicking on a cell, and cancel that by clicking anywhere else
                    if (data->cursorSwapping) {
                        toggleCursorSwapping(data);
                    }
                } else {
                    data->mouseClickPending = true;
                }
            }
        }
        data->mousePressPos.x = -1;
        data->mousePressPos.y = -1;
    }
}

// ----------
// OUT-OF-GAME FUNCTIONS
// (Game Over, Game Save)
// ----------

void gameOver(CrushData* data) {
    data->playState = CPS_GAME_OVER;
    data->gameOverPanel->visible = true;
    toggleHighScoreUI(data);
}

void saveGame(CrushData* data) {
    if (data->playState == CPS_GAME_OVER) {
        return;
    }

    char errMsg[256];
    if (boardSaveToFile(data->board, "savefile.pasteque", errMsg)) {
        uiDisplayNotification(&data->notificationData, "Partie sauvegardée !",
                              PASTEQUE_COLOR_WHITE_ON_DARK_GREEN, MICROS(5000));
    } else {
        uiDisplayNotification(&data->notificationData, errMsg, PASTEQUE_COLOR_WHITE_ON_RED, MICROS(5000));
    }
}

// ----------
// PAUSE UI
// ----------

void drawPauseUI(Panel* panel, PastequeGameState* gameState, void* panelData) {
    CrushData* data = panelData;
    PauseMenu* ui = &data->pauseUI;

    char* title = data->playState == CPS_GAME_OVER ? "Fin de la partie" : "Pause";
    char* mainStr = (data->playState == CPS_GAME_OVER && !data->storyModeActivated) ? "Rejouer" : "Continuer";
    char* secondStr = data->playState == CPS_GAME_OVER ? "Voir la grille" : "Sauvegarder";
    char* backStr = "Revenir au menu";

    for (int i = 0; i < panel->width; ++i) {
        ColorId color = i < 3 ? PASTEQUE_COLOR_BLACK : PASTEQUE_COLOR_WHITE;
        panelDrawLine(panel, 0, i, panel->width, ' ', color);
    }
    panelDrawTextCentered(panel, -1, 1, title, PASTEQUE_COLOR_BLACK);

    int btnWidth = panel->width - 2;
    uiDrawToggleOption(panel, &ui->state, &ui->mainButton, 1, 4, btnWidth, mainStr, 0, toggleStyleButton);
    uiDrawToggleOption(panel, &ui->state, &ui->secondaryButton, 1, 6, btnWidth, secondStr, 1, toggleStyleButton);
    uiDrawToggleOption(panel, &ui->state, &ui->quitButton, 1, 8, btnWidth, backStr, 2, toggleStyleButton);
}

void togglePause(CrushData* data) {
    if (data->playState != CPS_WAITING_INPUT && data->playState != CPS_GAME_OVER) {
        return;
    }
    bool newState = !data->paused;

    data->paused = newState;
    data->pauseUI.state.focused = newState;
    data->pauseUI.state.selectedIndex = 0;
    data->pausePanel->visible = newState;
}

// ----------
// HIGHSCORE UI
// ----------

ColorId rankColor(int rank) {
    switch (rank) {
        case 1: return PASTEQUE_COLOR_GOLD_BG;
        case 2: return PASTEQUE_COLOR_SILVER_BG;
        case 3: return PASTEQUE_COLOR_BRONZE_BG;
        default: return PASTEQUE_COLOR_DARK_PURPLE_BG;
    }
}

void drawHighscoreUI(Panel* panel, PastequeGameState* gameState, void* panelData) {
    CrushData* data = panelData;
    HighScoreDialog* ui = &data->highScoreUI;

    for (int i = 0; i < panel->height; ++i) {
        ColorId color = i < 3 ? PASTEQUE_COLOR_BLACK : PASTEQUE_COLOR_WHITE;
        panelDrawLine(panel, 0, i, panel->width, ' ', color);
    }
    panelDrawTextCentered(panel, -1, 1, "Entrez dans la légende !", PASTEQUE_COLOR_BLACK);

    ColorId rankCol = rankColor(ui->rank);
    // Draw the background of the banner
    for (int i = 4; i <= 6; ++i) {
        panelDrawLine(panel, 0, i, panel->width, ' ', rankCol);
    }

    char rankStr[32];
    if (ui->rank == 1) {
        snprintf(rankStr, 32, "Vous êtes 1er !");
    } else {
        snprintf(rankStr, 32, "Vous êtes %de !", ui->rank);
    }

    panelDrawTextCentered(panel, -1, 5, rankStr, rankCol);

    if (ui->rank > LEADERBOARD_MAX) {
        panelDrawTextCentered(panel, -1, 8, "Votre score est trop bas", PASTEQUE_COLOR_WHITE);
        panelDrawTextCentered(panel, -1, 9, "pour figurer dans le", PASTEQUE_COLOR_WHITE);
        panelDrawTextCentered(panel, -1, 10, "classement.", PASTEQUE_COLOR_WHITE);
        panelDrawTextCentered(panel, -1, 11, "Retentez votre chance !", PASTEQUE_COLOR_WHITE);
        uiDrawToggleOption(panel, &ui->state, &ui->skipButton, 1, 13, 27, "OK", 0, toggleStyleButton);
    } else {
        panelDrawTextCentered(panel, -1, 8, "Entrez votre nom pour faire", PASTEQUE_COLOR_WHITE);
        panelDrawTextCentered(panel, -1, 9, "partie du classement !", PASTEQUE_COLOR_WHITE);

        panelDrawText(panel, 2, 11, "Nom :", PASTEQUE_COLOR_WHITE);
        uiDrawTextInput(panel, &ui->state, &ui->nameInput, 8, 11, 19, 19, 0, textInputStyleDefault);

        uiDrawToggleOption(panel, &ui->state, &ui->confirmButton, 2, 13, 12, "Confirmer", 1, toggleStyleButton);
        uiDrawToggleOption(panel, &ui->state, &ui->skipButton, 15, 13, 12, "Passer", 2, toggleStyleButton);
    }
}

void toggleHighScoreUI(CrushData* data) {
    if (data->highScoreUI.state.focused) {
        data->highScoreUI.state.focused = false;
        data->highScorePanel->visible = false;
    } else {
        // Update the rank.
        data->highScoreUI.rank = hsRank("highscore.pasteque",
                                        data->board->score, data->board->symbols, data->board->sizePreset);
        data->highScoreUI.state.focused = true;
        data->highScorePanel->visible = true;
    }
}

void submitHighScore(CrushData* data) {
    HighScoreDialog* ui = &data->highScoreUI;
    char* name = ui->nameInput.inputText;
    if (strlen(name) == 0) {
        // Do nothing, name empty!
        return;
    }

    player hsPlayer;
    strncpy(hsPlayer.name, name, MAX_NAME_LENGTH);
    hsPlayer.score = data->board->score;
    hsPlayer.preset = data->board->sizePreset;
    hsPlayer.symbols = data->board->symbols;
    if (hsNew("highscore.pasteque", &hsPlayer)) {
        if (strcmp(name, "VOTRE NOM") == 0) {
            uiDisplayNotification(&data->notificationData, "MDRRRRRRR", PASTEQUE_COLOR_WHITE_ON_DARK_GREEN, MICROS(5000));
        }
        toggleHighScoreUI(data);
        togglePause(data);
    } else {
        uiDisplayNotification(&data->notificationData,
                              "Une erreur s'est produite pendant l'enregistrement du score dans le fichier.",
                              PASTEQUE_COLOR_WHITE_ON_RED, MICROS(5000));
    }
}

// ----------
// STORY
// ----------

void nextStory(CrushData* data, PastequeGameState* gameState) {
    gsSwitchScene(gameState, SN_STORY, makeStoryData(data->board->storyIndex + 1));
}

void crushStoryAutoSkip(CrushData* data, long micros) {
    if (data == NULL) {
        RAGE_QUIT(2500, "data is NULL");
    }

    if (data->storyModeActivated) {
        data->storyAutoSkip = true;
        data->storyAutoSkipTime = micros;
    }
}

// ----------
// GAME FUNCTIONS (Init, Event, Update, Draw, Finish)
// ----------

void crushInit(PastequeGameState* gameState, CrushData* data) {
    // Leave some space for horizontal spaces.
    PanelAdornment boardAdorn = makeAdornment(PAS_CLOSE_BORDER, PASTEQUE_COLOR_WHITE);
    data->boardPanel = gsAddPanel(gameState, 2, 2, 2 * data->board->width - 1, data->board->height,
                                  boardAdorn, &drawBoardPanel, data);

    data->scorePanel = gsAddPanel(gameState, 2, data->boardPanel->y + data->boardPanel->height + 1, 32, 1,
                                  noneAdornment, &drawScorePanel, data);

    data->gameOverPanel = gsAddPanel(gameState, 2, data->scorePanel->y + 2, 20, 5,
                                     makeAdornment(PAS_CLOSE_BORDER, PASTEQUE_COLOR_ORANGE), &drawGameOverPanel, data);

    data->gameOverPanel->visible = false;

    data->comboPanel = gsAddPanel(gameState, data->boardPanel->x + data->boardPanel->width + 2, 2,
                                  60, 60, noneAdornment, &drawComboPanel, data);

    data->pausePanel = gsAddPanel(gameState, 0, 0, 22, 10, boardAdorn, &drawPauseUI, data);
    data->pausePanel->adornment.colorPairOverrideV = PASTEQUE_COLOR_WHITE_ON_WHITE;
    data->pausePanel->adornment.colorPairOverrideEndY = 2;
    data->pausePanel->visible = false;
    panelCenterScreen(data->pausePanel, true, true);
    gsMovePanelLayer(gameState, data->pausePanel, 1);

    data->highScorePanel = gsAddPanel(gameState, 0, 0, 29, 15, boardAdorn, &drawHighscoreUI, data);
    data->highScorePanel->adornment.colorPairOverrideV = PASTEQUE_COLOR_WHITE_ON_WHITE;
    data->highScorePanel->adornment.colorPairOverrideEndY = 2;
    data->highScorePanel->visible = false;
    panelCenterScreen(data->highScorePanel, true, true);
    gsMovePanelLayer(gameState, data->highScorePanel, 2);

    uiAddNotificationPanel(gameState, 40, &data->notificationData);

    data->playState = CPS_EVALUATING_BOARD;
}

void crushUpdate(PastequeGameState* gameState, CrushData* data, unsigned long deltaTime) {
    if (!data->paused) {
        data->board->playTime += deltaTime;
        if (data->storyAutoSkip && data->board->playTime >= data->storyAutoSkipTime) {
            nextStory(data, gameState);
            return;
        }

        if (data->playState == CPS_EVALUATING_BOARD) {
            bool markedSomething = boardMarkAlignedCells(data->board);

            if (markedSomething) {
                data->playState = CPS_BLINKING;
            } else if (boardAnySwapPossible(data->board)) {
                boardResetCombo(data->board);
                data->playState = CPS_WAITING_INPUT;
            } else {
                boardResetCombo(data->board);
                gameOver(data);
            }

        } else if (data->playState == CPS_BLINKING) {
            // Blinking color is handled in drawBoardPanel.
            data->animTimeMicros += deltaTime;
            if (data->animTimeMicros >= BLINKING_DURATION_MICROS) {
                data->animTimeMicros = 0;

                // Destroy all marked cells after blinking.
                boardDestroyMarked(data->board);
                data->playState = CPS_GRAVITY;
            }
        }
        if (data->playState == CPS_GRAVITY) {
            data->animTimeMicros += deltaTime;
            if (data->animTimeMicros >= GRAVITY_PERIOD_MICROS) {
                data->animTimeMicros = 0;

                bool gravityDidSomething = boardGravityTick(data->board);
                if (gravityDidSomething) {
                    // Continue once we reach GRAVITY_PERIOD_MICROS again.
                } else {
                    // Reevaluate the board if we have any chain reaction.
                    data->playState = CPS_EVALUATING_BOARD;
                }
            }
        }
    }

    // Make sure our modal panels are centered if the window size changes
    panelCenterScreen(data->pausePanel, true, true);
    panelCenterScreen(data->highScorePanel, true, true);

    uiUpdateNotificationPanel(&data->notificationData, deltaTime);
}

void crushEvent(PastequeGameState* gameState, CrushData* data, Event* pEvent) {
    // Move the cursor according to input method settings
    const bool isZQSD = (data->inputMethod & CIM_ZQSD_SPACE_CURSOR) != 0;
    const bool isArrows = (data->inputMethod & CIM_ARROWS_ENTER_CURSOR) != 0;
    const bool isMouse = (data->inputMethod & CIM_MOUSE) != 0;
    const KeyCode code = pEvent->code;
    PauseMenu* pauseUI = &data->pauseUI;
    HighScoreDialog* hsUI = &data->highScoreUI;

    if (pauseUI->state.focused) {
        UINavBlock blocks[] = {{0, 2, ND_VERTICAL}};
        uiKeyboardNav(&pauseUI->state, pEvent, blocks, 1);

        if (pEvent->code == KEY_S && data->storyModeActivated) {
            // Temporary cheat: go to next story
            nextStory(data, gameState);
            return;
        }

        if (uiHandleToggleOptionEvent(&pauseUI->state, &pauseUI->mainButton, pEvent)) {
            if (data->playState == CPS_GAME_OVER) {
                if (data->storyModeActivated) {
                    // Continue the story
                    nextStory(data, gameState);
                } else {
                    // Play again
                    CrushBoard* orig = data->board;
                    CrushBoard* board = makeCrushBoard(orig->sizePreset, orig->width, orig->height, orig->symbols);
                    gsSwitchScene(gameState, SN_CRUSH, makeCrushData(board, data->inputMethod));
                }
                return;
            } else {
                // Continue
                togglePause(data);
            }
        } else if (uiHandleToggleOptionEvent(&pauseUI->state, &pauseUI->secondaryButton, pEvent)) {
            if (data->playState == CPS_GAME_OVER) {
                // Contemple
                togglePause(data);
            } else {
                // Save
                saveGame(data);
                togglePause(data);
            }
        } else if (uiHandleToggleOptionEvent(&pauseUI->state, &pauseUI->quitButton, pEvent)) {
            gsSwitchScene(gameState, SN_MAIN_MENU, makeMainMenuData());
        }
    } else if (hsUI->state.focused) { ;
        if (uiHandleTextInputEvent(&hsUI->state, &hsUI->nameInput, pEvent)) {
            return;
        } else if (uiHandleToggleOptionEvent(&hsUI->state, &hsUI->confirmButton, pEvent)) {
            submitHighScore(data);
            return;
        } else if (uiHandleToggleOptionEvent(&hsUI->state, &hsUI->skipButton, pEvent)) {
            toggleHighScoreUI(data);
            togglePause(data);
            return;
        }

        if (hsUI->rank > LEADERBOARD_MAX) {
            // We only have one button: OK (as skipButton)
            UINavBlock blocks[] = {{0, 0, ND_VERTICAL}};
            uiKeyboardNav(&hsUI->state, pEvent, blocks, 1);
        } else {
            // Normal UI.
            UINavBlock blocks[] = {
                    {0, 0, ND_VERTICAL},
                    {1, 2, ND_HORIZONTAL}
            };
            uiKeyboardNav(&hsUI->state, pEvent, blocks, 2);
        }
    } else {
        if (data->playState == CPS_WAITING_INPUT) {
            if (isZQSD && code == KEY_Z || isArrows && code == KEY_UP) {
                runCursorMoveAction(data, 0, -1);
            } else if (isZQSD && code == KEY_S || isArrows && code == KEY_DOWN) {
                runCursorMoveAction(data, 0, 1);
            } else if (isZQSD && code == KEY_Q || isArrows && code == KEY_LEFT) {
                runCursorMoveAction(data, -1, 0);
            } else if (isZQSD && code == KEY_D || isArrows && code == KEY_RIGHT) {
                runCursorMoveAction(data, 1, 0);
            } else if (isZQSD && code == KEY_SPACE || isArrows && code == KEY_RETURN) {
                toggleCursorSwapping(data);
            } else if (isMouse && code == KEY_MOUSE) {
                MEVENT mouse = pEvent->mouseEvent;
                if (!data->cursorSwapping) {
                    registerMouseCursorMove(data, mouse.x, mouse.y);
                }
                if ((pEvent->mouseEvent.bstate & (BUTTON1_PRESSED | BUTTON3_PRESSED)) != 0) {
                    registerMousePressed(data, mouse.x, mouse.y);
                } else if ((pEvent->mouseEvent.bstate & (BUTTON1_RELEASED | BUTTON3_RELEASED)) != 0) {
                    registerMouseReleased(data, mouse.x, mouse.y);
                }
            }
        }
        if (code == KEY_P) {
            togglePause(data);
        }
        if (code == KEY_N) {
            saveGame(data);
        }
    }
}

void crushDrawBackground(PastequeGameState* gameState, CrushData* data, Screen* pScreen) {
}

void crushDrawForeground(PastequeGameState* gameState, CrushData* data, Screen* pScreen) {

}

void crushFinish(PastequeGameState* gameState, CrushData* data) {
    if (data->board) {
        free(data->board);
    }
}

