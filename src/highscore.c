#include "highscore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void scores_sort(player *players, int count){
    int etape,decal;
    player temp;
    for (etape = 1; etape< count; etape++){
        temp = players[etape];
        decal=etape-1;
        while (decal>=0 && players[decal].score<temp.score){
            players[decal+1]=players[decal];
            decal--;
        }
        players[decal+1]=temp;
    }
}

void ajustscores(const char* filename, player* players, int count){
    FILE* file = fopen(filename, "w");  // Open the file in write mode
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return;
    }
    for (int i = 0; i < count; i++){
        fprintf(file, "Score : %d by %s using the BoardSizePreset %d playing with %hhd symbols\n", players[i].score, players[i].name,players[i].BoardSizePreset,players[i].symbols);
    }
    

    fclose(file);
}

int removeDuplicatePlayers(player* players, int playerCount) {
    int i, j;
    for (i = 0; i < playerCount - 1; i++) {
        if (strcmp(players[i].name, players[i + 1].name) == 0 && players[i].BoardSizePreset == players[i + 1].BoardSizePreset && players[i].symbols == players[i + 1].symbols) {
            if (players[i].score <= players[i + 1].score) {
                // Shift elements to the left to remove player i
                for (j = i; j < playerCount - 1; j++) {
                    strcpy(players[j].name, players[j + 1].name);
                    players[j].score = players[j + 1].score;
                    players[j].BoardSizePreset = players[j + 1].BoardSizePreset;
                    players[j].symbols = players[j + 1].symbols;
                }
                (playerCount)--;
                i--;  // Revisit the current index after shifting elements
            } else {
                // Shift elements to the left to remove the player i+1
                for (j = i + 1; j < playerCount - 1; j++) {
                    strcpy(players[j].name, players[j + 1].name);
                    players[j].score = players[j + 1].score;
                    players[j].BoardSizePreset = players[j + 1].BoardSizePreset;
                    players[j].symbols = players[j + 1].symbols;
                }
                (playerCount)--;
            }
        }
    }
    return playerCount;
}


void player_display(player *players, int top){
    printf("Top %d players:\n",top);
    for (int i = 0; i < top; i++) {
        printf("Name: %s, Score: %d bsp : %d, symbols : %c\n", players[i].name, players[i].score,players[i].BoardSizePreset,players[i].symbols);
    }
    
}

void new_highscore(const char* filename, const player* newPlayer) {
    FILE* file = fopen(filename, "a");  // Open the file in append mode
    if (file == NULL) {
        printf("Failed to open the file.\n");
    }

    fprintf(file, "\nScore : %d by %s using the BoardSizePreset %d playing with %hhd symbols\n", newPlayer->score, newPlayer->name,newPlayer->BoardSizePreset,newPlayer->symbols);

    fclose(file);
}


bool parseFile(const char* filename, player* players, int maxPlayers, int* outNumPlayers) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        *outNumPlayers = 0;
        return false;
    }

    int count = 0;
    char line[256];  // Assuming the line won't exceed the buffer size

    while (fgets(line, sizeof(line), file) != NULL && count < maxPlayers) {
        // Parse the name and score from the line
        char name[MAX_NAME_LENGTH];
        int score;
        int BoardSizePreset;
        char symbols;
        if (sscanf(line, "Score : %d by %s using the BoardSizePreset %d playing with %hhd symbols", &score, name, &BoardSizePreset, &symbols) != 4) {
            continue;  // Skip this line and continue to the next one
        }

        // Stores the informations in the players array
        strncpy(players[count].name, name, MAX_NAME_LENGTH);
        players[count].score = score;
        players[count].BoardSizePreset = BoardSizePreset;
        players[count].symbols = symbols;
        count++;
    }
    scores_sort(players,count); //sorts the players accordingly to their score
    count = removeDuplicatePlayers(players, count); //remove the worst score if the player already had one 

    fclose(file);
    ajustscores(filename,players,count<100?count:100);

    *outNumPlayers = count;
    return true;
}
