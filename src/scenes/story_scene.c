#include "scenes/story_scene.h"
#include "story_scene_text.h"
#include "colors.h"
#include "board.h"
#include "scenes/crush_scene.h"
#include "scenes/main_menu_scene.h"
#include <malloc.h>
#include <string.h>
#include <ctype.h>

#define TEXT_SIZE 6000
#define MAX_MARKUPS 100
#define CHARACTER_PERIOD MICROS(36)
#define PAUSE_DURATION MICROS(300)

typedef enum {
    MK_USER_CONTINUE = 1,
    MK_PAUSE = 2,
    MK_NARRATOR_COLOR = 4,
    MK_STORY_COLOR = 8
} MarkupKind;

typedef struct {
    int index;
    MarkupKind kind;
} Markup;

struct StoryData_S {
    int index;

    const char* rawText;
    char storyText[TEXT_SIZE];
    int storyLen;
    int storyCursor;
    char displayedText[TEXT_SIZE];
    int displayedCursor;
    Markup markups[MAX_MARKUPS];
    int curMarkupIndex;
    long characterTimer;

    bool waitUserContinue;
    bool storyEnded;
    long pauseTimer; // -1 = no pause
    ColorId color;

    Panel* textPanel;
    Panel* continuePanel;
};

const int stories = 12;

StoryData* makeStoryData(int storyIndex) {
    if (storyIndex < 0) {
        RAGE_QUIT(80000, "Story index out of bounds (%d)", storyIndex);
    }
    if (storyIndex >= stories) {
        // A very high number will lead to the last story.
        storyIndex = stories - 1;
    }
    StoryData* data = calloc(1, sizeof(StoryData));
    if (data == NULL) {
        RAGE_QUIT(80001, "Failed to allocate StoryData.");
    }
    data->index = storyIndex;
    return data;
}

void drawTextPanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    StoryData* data = panelData;

    panelDrawText(panel, 0, 0, data->displayedText, data->color);
}

void drawContinuePanel(Panel* panel, PastequeGameState* gameState, void* panelData) {
    StoryData* data = panelData;

    if (data->waitUserContinue) {
        panelDrawTextCentered(panel, -1, 0, "Appuyez sur une touche pour voir la suite", PASTEQUE_COLOR_TURQUOISE);
    }
}

void initText(StoryData* data) {
    data->rawText = storyTexts[data->index];

    // Strip all formatting tags from the text-wrapped text.
    // Also ignores markups while wrapping
    char wrapped[TEXT_SIZE];
    panelWrapText(data->rawText, data->textPanel->width, true, wrapped, TEXT_SIZE);
    int iSto = 0;
    int iMark = 0;
    int iWrap = 0;

    while (wrapped[iWrap] != '\0') {
        if (wrapped[iWrap] == '/') {
            // Markup!
            MarkupKind kind;
            switch (wrapped[iWrap + 1]) {
                case 'C':
                    kind = MK_USER_CONTINUE;
                    break;
                case 'P':
                    kind = MK_PAUSE;
                    break;
                case 'S':
                    kind = MK_STORY_COLOR;
                    break;
                case 'N':
                    kind = MK_NARRATOR_COLOR;
                    break;
                default:
                    RAGE_QUIT(80010, "Unknown markup sequence: /%c", wrapped[iWrap + 1]);
                    return;
            }
            if (iMark > 0 && data->markups[iMark - 1].index == iSto) {
                // In case we're stacking multiple markups on the same character.
                data->markups[iMark - 1].kind |= kind;
            } else {
                Markup m = {iSto, kind};
                data->markups[iMark] = m;
                iMark++;
            }

            // Don't add the character
            iWrap += 2;
        } else {
            data->storyText[iSto] = wrapped[iWrap];
            iSto++;
            iWrap++;
        }
    }

    data->storyText[iSto] = '\0';
    data->storyLen = iSto;
    data->color = PASTEQUE_COLOR_WHITE;
}

void applyMarkup(StoryData* data, MarkupKind markup) {
    if ((markup & MK_USER_CONTINUE) != 0) {
        data->waitUserContinue = true;
    }
    if ((markup & MK_PAUSE) != 0) {
        data->pauseTimer = PAUSE_DURATION;
    }
    if ((markup & MK_NARRATOR_COLOR) != 0) {
        data->color = PASTEQUE_COLOR_ORANGE;
    }
    if ((markup & MK_STORY_COLOR) != 0) {
        data->color = PASTEQUE_COLOR_WHITE;
    }
}

int getUTF8Length(char ch) {
    // See https://en.wikipedia.org/wiki/UTF-8#Encoding
    unsigned char cp2 = (3 << 6); // 0b11000000
    unsigned char cp3 = (7 << 5); // 0b11100000
    unsigned char cp4 = (15 << 4); // 0b11110000
    unsigned char chU = (unsigned char) ch;

    if ((chU & cp4) == cp4) {
        return 4;
    } else if ((chU & cp3) == cp3) {
        return 3;
    } else if ((chU & cp2) == cp2) {
        return 2;
    } else {
        return 1;
    }
}

void tickText(StoryData* data, unsigned long deltaTime) {
    if (data->waitUserContinue || data->storyEnded) {
        return;
    } else if (data->pauseTimer != -1) {
        data->pauseTimer -= deltaTime;
        if (data->pauseTimer <= 0) {
            data->pauseTimer = -1;
        }
        return;
    }

    data->characterTimer += deltaTime;
    int chars = data->characterTimer / CHARACTER_PERIOD;
    data->characterTimer -= CHARACTER_PERIOD * chars;

    for (int i = 0; i < chars; ++i) {
        if (data->storyCursor >= data->storyLen) {
            data->storyEnded = true;
            data->waitUserContinue = true;
            break;
        }

        if (data->markups[data->curMarkupIndex].index == data->storyCursor) {
            if (data->curMarkupIndex >= MAX_MARKUPS) {
                RAGE_QUIT(81000, "Too much markups!");
            }
            applyMarkup(data, data->markups[data->curMarkupIndex].kind);
            data->curMarkupIndex++;
        }

        // Skip newlines at the beginning of the text.
        bool skipChar = data->displayedCursor == 0 && isspace(data->storyText[data->storyCursor]);

        int utfLen = getUTF8Length(data->storyText[data->storyCursor]);
        if (!skipChar) {
            for (int j = 0; j < utfLen; ++j) {
                data->displayedText[data->displayedCursor + j] = data->storyText[data->storyCursor + j];
            }
            data->displayedCursor += utfLen;
            data->displayedText[data->displayedCursor] = '\0';
        }
        data->storyCursor += utfLen;
    }
}

void continueText(StoryData* data, PastequeGameState* gameState) {
    if (data->waitUserContinue) {
        if (data->storyEnded) {
            if (data->index == stories - 1) {
                // The END! What do?
                gsSwitchScene(gameState, SN_MAIN_MENU, makeMainMenuData());
            } else {
                // Switch to a game, woo!
                CrushBoard* board = makeCrushBoard(storyPresets[data->index], 0, 0, 4);
                board->storyIndex = data->index;

                CrushData* scene = makeCrushData(board, CIM_ALL);
                if (data->index == 4) {
                    // Special prank for Chapter 5
                    crushStoryAutoSkip(scene, MICROS(6000));
                }
                if (data->index == 7) {
                    // Chapter 8: Time travel machine (25s)
                    crushStoryAutoSkip(scene, MICROS(25000));
                }

                gsSwitchScene(gameState, SN_CRUSH, scene);
            }
        } else {
            memset(data->displayedText, 0, sizeof(char) * TEXT_SIZE);
            data->displayedCursor = 0;
            data->waitUserContinue = false;
        }
    }
}

void storyInit(PastequeGameState* gameState, StoryData* data) {
    data->textPanel = gsAddPanel(gameState, 0, 0, 90, 15, noneAdornment, &drawTextPanel, data);
    data->continuePanel = gsAddPanel(gameState, 0, 0, 90, 1, noneAdornment, &drawContinuePanel, data);

    initText(data);
}

void storyUpdate(PastequeGameState* gameState, StoryData* data, unsigned long deltaTime) {
    tickText(data, deltaTime);

    panelCenterScreen(data->textPanel, true, true);
    panelCenterScreen(data->continuePanel, true, false);
    data->continuePanel->y = data->textPanel->y + data->textPanel->height + 2;
}

void storyEvent(PastequeGameState* gameState, StoryData* data, Event* pEvent) {
    if (data->waitUserContinue && (pEvent->code != KEY_MOUSE && pEvent->code != KEY_RESIZE)) {
        continueText(data, gameState);
    }

    if (pEvent->code == KEY_S && gameState->enableCheats) {
        // Cheat: skip story
        data->waitUserContinue = true;
        data->storyEnded = true;
        continueText(data, gameState);
    }
}

void storyFinish(PastequeGameState* gameState, StoryData* data) {}
