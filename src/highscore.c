#include "highscore.h"
#include "libGameRGR2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//You know how leaderboards work right ?
void scores_sort(player* players, int count) {
    int etape, decal;
    player temp;
    for (etape = 1; etape < count; etape++) {
        temp = players[etape];
        decal = etape - 1;
        while (decal >= 0 && players[decal].score < temp.score) {
            players[decal + 1] = players[decal];
            decal--;
        }
        players[decal + 1] = temp;
    }
}

// Rewrites the file with only the best scores for each category (see LEADERBOARD_MAX)
// Can also change the size of the array by removing some of its elements.
// Expects the players array to be sorted.
void ajustscores(const char* filename, player* players, int* count) {
    FILE* file = fopen(filename, "w");  // Open the file in write mode
    if (file == NULL) {
        // Then the file is likely readable, but write-protected.
        // Maybe that there should be something shown on screen for this,
        // but it's not so important so let's not complicate it.
    }

    // To avoid a very costly operation with three for loops, instead have
    // an array that stores each unique leaderboard configuration (symbol & size)
    // using a unique identifier of 4 bits:
    //                      ss|pp
    // symbols 4+(0 to 2) --^^|^^-- preset (0 to 3)
    int leaderboardsCount[16] = {0}; // The number of players in each leaderboard

    for (int i = 0; i < *count; ++i) {
        // Move the symbols count two bits on the left: [ss]|pp
        // Add the preset bits on the right: ss|[pp]
        int id = (players[i].symbols - 4) << 2 | players[i].preset;
        if (leaderboardsCount[id] != LEADERBOARD_MAX) {
            if (file) {
                fprintf(file, "Score : %d by %s using the BoardSizePreset %d playing with %hhd symbols\n",
                        players[i].score, players[i].name, players[i].preset, players[i].symbols);
            }

            leaderboardsCount[id]++;
        } else {
            // Remove the player from the array: shift all elements next to it.
            // Operation cost: O(n) but that's rare anyway right? Unless someone
            // decides to violently add 2000 times the same line to the file...
            for (int j = i; j < *count - 1; ++j) {
                players[i] = players[i + 1];
            }
            (*count)--;
            i--; // Adjust the index since we have moved forward.
        }
    }

    if (file) {
        fclose(file);
    }
}

bool hsNew(const char* filename, const player* newPlayer) {
    FILE* file = fopen(filename, "a");  // Open the file in append mode
    if (file == NULL) {
        return false;
    }

    // Replace spaces with ~ (tilde).
    // Do some extra checks if the string is not null-terminated.
    char name[MAX_NAME_LENGTH];
    strncpy(name, newPlayer->name, MAX_NAME_LENGTH - 1);
    name[MAX_NAME_LENGTH - 1] = '\0';
    for (int i = 0; name[i] != '\0'; ++i) {
        if (isspace(name[i])) {
            name[i] = '~';
        }
    }

    bool success;
    success = fprintf(file, "\nScore : %d by %s using the BoardSizePreset %d playing with %hhd symbols\n",
                      newPlayer->score, name, newPlayer->preset, newPlayer->symbols);

    fclose(file);

    return success;
}


bool hsParse(const char* filename, player* players, int* outNumPlayers) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        *outNumPlayers = 0;
        return false;
    }

    int count = 0;
    char line[256];  // Assuming the line won't exceed the buffer size

    while (fgets(line, sizeof(line), file) != NULL && count < MAX_PLAYERS) {
        // Parse the name and score from the line
        char name[MAX_NAME_LENGTH];
        int score;
        int preset;
        char symbols;
        // %29s limits the string to 29 characters.
        if (sscanf(line, "Score : %d by %29s using the BoardSizePreset %d playing with %hhd symbols",
                   &score, name, &preset, &symbols) != 4) {
            continue;  // Skip this line and continue to the next one
        }

        // Replace ~ (tilde) with spaces
        for (int i = 0; name[i] != '\0'; ++i) {
            if (name[i] == '~') {
                name[i] = ' ';
            }
        }
        // Stores the informations in the players array
        strcpy(players[count].name, name);
        players[count].score = score;
        players[count].preset = preset;
        players[count].symbols = symbols;
        count++;
    }
    scores_sort(players, count); //sorts the players accordingly to their score

    fclose(file);
    ajustscores(filename, players, &count);

    *outNumPlayers = count;
    return true;
}

int hsRank(const char* filename, int score, char symbols, int preset) {
    player players[MAX_PLAYERS];
    int numPlayers;
    if (!hsParse(filename, players, &numPlayers)) {
        // Most likely, the file doesn't exist.
        return 1;
    }

    // Go through all the scores in the leaderboard (they are sorted from greatest to lowest),
    // increase the rank until we see a lower or equal score.
    int rank = 1;

    for (int i = 0; i < numPlayers; ++i) {
        player* pl = &players[i];
        if (pl->preset == preset && pl->symbols == symbols) {
            if (pl->score > score) {
                rank++;
            } else {
                break;
            }
        }
    }

    return rank;
}
