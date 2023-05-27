/**
 * highscore.h
 * -------------
 * Contains everything needed to store the highscore so you can brag about it
 */
#include <stdbool.h>

// The maximum amount of players in a specific high score leaderboard
// (Example: 4 symbols, medium size)
#define LEADERBOARD_MAX 12
// ALL COMBINATIONS OF:
// - 4, 5, 6 symbols
// - small, medium, large, custom board
// = 4*3=12
#define MAX_PLAYERS LEADERBOARD_MAX*12
#define MAX_NAME_LENGTH 30

typedef struct {
    char name[MAX_NAME_LENGTH];
    int score;
    int preset;
    char symbols;
} player;

// Adds a new high score entry to the given file
// Returns true when the file has been written successfully.
bool hsNew(const char* filename, const player* newPlayer);

// main function to read the file, rank the players and rewrite the file
// outNumPlayers will contain the number of players in the array
// Returns true when the file has been parsed successfully.
bool hsParse(const char* filename, player* players, int* outNumPlayers);

// Returns the rank where a player would be with their score,
// depending on their board (symbol and preset). If the leaderboard is
// empty, returns 1 (the first place).
// Also returns 1 on error.
int hsRank(const char* filename, int score, char symbols, int preset);