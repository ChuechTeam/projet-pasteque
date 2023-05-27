#ifndef PROJET_PASTEQUE_STORY_SCENE_H
#define PROJET_PASTEQUE_STORY_SCENE_H

#include "game_state.h"

struct StoryData_S;
typedef struct StoryData_S StoryData;

extern const int stories;

StoryData* makeStoryData(int storyIndex);

void storyInit(PastequeGameState* gameState, StoryData* data);

void storyUpdate(PastequeGameState* gameState, StoryData* data, unsigned long deltaTime);

void storyEvent(PastequeGameState* gameState, StoryData* data, Event* pEvent);

void storyFinish(PastequeGameState* gameState, StoryData* data);

#endif //PROJET_PASTEQUE_STORY_SCENE_H
