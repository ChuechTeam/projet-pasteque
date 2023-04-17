#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAILLE 10

typedef char matrice[TAILLE][TAILLE];

void print_color(char ch) { //prints the colored character 
    switch(ch) {
        /*case 0: // black
            printf("\033[0;30m%c \033[0m", ch);
            break;
        case 1: // red
            printf("\033[0;31m%c \033[0m", ch);
            break;
        case 2: // green
            printf("\033[0;32m%c \033[0m", ch);
            break;
        case 3: // yellow
            printf("\033[0;33m%c \033[0m", ch);
            break;
        case 4: // blue
            printf("\033[0;34m%c \033[0m", ch);
            break;
        case 5: // magenta
            printf("\033[0;35m%c \033[0m", ch);
            break;
        case 6: // cyan
            printf("\033[0;36m%c \033[0m", ch);
            break;
        case 7: // white
            printf("\033[0;37m%c \033[0m", ch);
            break;
        */
       case 'X': // red
            printf("\033[0;31m%c \033[0m", ch);
            break;
        case 'O': // green
            printf("\033[0;32m%c \033[0m", ch);
            break;
        case '&': // blue
            printf("\033[0;34m%c \033[0m", ch);
            break;
        case '#': // cyan
            printf("\033[0;36m%c \033[0m", ch);
            break;
        case '$': // yellow
            printf("\033[0;33m%c \033[0m", ch);
            break;
        default: // default color
            printf("%c ", ch);
            break;
    }
}

char fill(){
    int random;
    random = rand()%5; 
    switch (random){
        case 0:
            return 'X';
            break;
        case 1:
            return 'O';
            break;
        case 2:
            return '&';
            break;
        case 3:
            return '#';
            break;
        case 4:
            return '$';
            break;
        default:
            return '0';
            break;
    }
}

void ini(matrice tab){ // Fills the board initially
    srand(time(NULL));
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            tab[i][j]=fill();
        }
    } 
}

void destroy(matrice tab, int i, int j){ //replaces all tiles above the destoyed tiles
    for(i; i>0 ;i--){
        tab[i][j]=tab[i-1][j];
    }
    tab[0][j] = fill(); //creates a new tiles at the top
}

void play(matrice tab){

}

void start(matrice tab){ // Initializes the board so 3 symbols are not connected
     
}

void boardprint(matrice tab) {
    printf("   Welcome to CY Crush !\n\n");
    printf("    A B C D E F G H I J\n");
    for (int i = 0; i < TAILLE; i++) {
        printf("%d | ",i);
        for (int j = 0; j < TAILLE; j++) {
            print_color(tab[i][j]); //prints with the color
        }
        printf("|\n");
    }
}

int main() {
    char tableau[TAILLE][TAILLE];
    ini(tableau);
    boardprint(tableau);
    return 0;
}
