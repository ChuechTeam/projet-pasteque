/**
 * highscore.h
 * -------------
 * Contains everything needed to store the highscore so you can brag about it
 */

#define MAX_PLAYERS 101
#define MAX_NAME_LENGTH 100 

typedef struct {
    char name[MAX_NAME_LENGTH];
    int score;
    int BoardSizePreset;
    char symbols; 
} player;

//You know how leaderboards work right ?
void scores_sort(player *players, int count); 

void new_highscore(const char* filename, const player* newPlayer);

//Rewrites the files with the 100 (or less) best scores
void ajustscores(const char* filename, player* players, int count);

int removeDuplicatePlayers(player* players, int playerCount);

 //main function to read the file, rank the players and rewrite the file
void parseFile(const char* filename, player* players, int maxPlayers);
    
void player_display(player *players, int top);