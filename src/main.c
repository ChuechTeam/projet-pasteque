#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAILLE 10

typedef char matrice[TAILLE][TAILLE];

typedef struct{
    int y;
    int x;
}Coos;

void insertion(Coos *tab, int n){
    int etape,decal;
    Coos temp;
    for (etape = 1; etape< n; etape++){
        temp = tab[etape];
        decal=etape-1;
        while (decal>=0 && tab[decal].y<temp.y){
            tab[decal+1]=tab[decal];
            decal--;
        }
        tab[decal+1]=temp;
    }
}

void afficheCoo(Coos cord){
    printf("\n%d %d",cord.y,cord.x);
}

void printArray(Coos *tab){
    for (int i = 0 ; i < TAILLE*TAILLE ; i++){
        afficheCoo(tab[i]);
    }
}

void initiaCoos(Coos *tab){
    for (int i = 0 ; i < TAILLE*TAILLE ; i++){
        tab[i].x = 20;
        tab[i].y = 20;
    }
}

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

void tri_coo(Coos *tab,int n){

}

void destroy(matrice tab, int i, int j){ //replaces all tiles above the destoyed tiles
    for(i; i<TAILLE-1 ;i++){
        tab[i][j]=tab[i+1][j];
    }
    tab[TAILLE-1][j] = fill(); //creates a new tile at the bottom
}

void board_destroy(matrice tab, int n, Coos *cord){
    for (int i = 0; i < n; i++){
        destroy(tab,cord[i].y,cord[i].x);
    }
    
}

int points(int lenght){
    switch (lenght){
    case 3:
        return 300;
        break;
    case 4:
        return 450;
        break;
    case 5:
        return 650;
        break;
    case 6:
        return 900;
        break;
    case 7:
        return 1500;
        break;
    case 8:
        return 2500;
        break;
    case 9:
        return 4000;
        break;
    case 10:
        return 10000;
        break;
    default:
        return 0;
        break;
    }
}

void play(matrice tab, Coos *coordonnees, int *score){
    int lenght = 1;
    int flag = 1;
    int n = 0;
    //Check for lines
    //Checks bacwards and if stops go fwd
    /*for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            if(flag){
                if(tab[i][0]==tab[i][TAILLE-1-j]){
                    lenght++;
                }
                else{

                }
            }
        }
    }*/
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE-1; j++){
            flag = 1;
            if(j==TAILLE-2){
                if(tab[i][j]==tab[i][j+1]){
                    lenght ++;
                    if(lenght>2){
                        for (int k = -1; k < lenght; k++){
                            coordonnees[n].y = i;
                            coordonnees[n].x = j-k;
                            n++;
                        }
                        *score+=points(lenght);
                        lenght = 1;
                    }
                }
                else if(lenght>2){
                    for (int k = 0; k < lenght; k++){
                        coordonnees[n].y = i;
                        coordonnees[n].x = j-k;
                        n++;
                    }
                    *score+=points(lenght);
                    lenght=1;
                }
            }
            else{
                if(tab[i][j]==tab[i][j+1]){
                    lenght ++;
                    flag = 0;
                }
                if(flag){
                    if(lenght>2){
                        for (int k = 0; k < lenght; k++){
                            coordonnees[n].y = i;
                            coordonnees[n].x = j-k;
                            n++;
                        }
                        *score+=points(lenght);
                        lenght = 1;
                    }
                    else{
                        lenght = 1;
                    } 
                }
            }
        }
        lenght = 1;
    }
    
    //check for colums
    for (int j = 0; j < TAILLE; j++){
        for (int i = 0; i < TAILLE-1; i++){
            flag = 1;
            if(i==TAILLE-2){
                if(tab[i][j]==tab[i+1][j]){
                    lenght ++;
                    if(lenght>2){
                        for (int k = -1; k < lenght; k++){
                            coordonnees[n].y = i-k;
                            coordonnees[n].x = j;
                            n++;
                        }
                        *score+=points(lenght);
                        lenght = 1;
                    }
                }
                else if(lenght>2){
                    for (int k = 0; k < lenght; k++){
                        coordonnees[n].y = i-k;
                        coordonnees[n].x = j;
                        n++;
                    }
                    *score+=points(lenght);
                    lenght=1;
                }
            }
            else{
                if(tab[i][j]==tab[i+1][j]){
                    lenght ++;
                    flag = 0;
                }
                if(flag){
                    if(lenght>2){
                        for (int k = 0; k < lenght; k++){
                            //destroy(tab,i-k,j);
                            coordonnees[n].y = i-k;
                            coordonnees[n].x = j;
                            n++;
                        }
                        *score+=points(lenght);
                        lenght = 1;
                    }
                    else{
                        lenght = 1;
                    } 
                }
            }
        }
        lenght = 1;
    }
    insertion(coordonnees,n);
    //clignote(tab, coos, n) (to be done) 
    board_destroy(tab, n, coordonnees);
}

void start(matrice tab){ // Initializes the board so 3 symbols are not connected
     
}

void boardprint(matrice tab,int score) {
    printf("   Welcome to CY Crush !\n                              You have scored %d points !\n",score);
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
    int score = 0;
    int* pscore = &score;
    char tableau[TAILLE][TAILLE];
    Coos *coordonnees = malloc(sizeof(Coos)*TAILLE*TAILLE);
    ini(tableau);
    boardprint(tableau,score);
    play(tableau,coordonnees, pscore);
    score = 0; //score during the steup obviously won't count towards the player's score
    boardprint(tableau,score);
    return 0;
}
