# Projet Pasteque
Un jeu de style Candy Crush, sur terminal.

## Présentation du jeu
### Règles du jeu
Le but du jeu est d'aligner 3 cases (ou plus) de même symbole en échangeant 
de place deux cases adjacentes. Il y a quelques particularités par rapport
à des Match-3 classiques :
* Les lignes peuvent traverser les bords de la grille
* D'autres symboles n'apparaissent pas après avoir détruit des symboles
* La gravité est dirigée vers le haut

Vous pouvez démarrer une partie avec 4, 5 ou 6 symboles, et choisir une
taille de grille :
* Petite (10x7)
* Moyenne (15x10)
* Large (24x16)
* Personnalisée : largeur et hauteur comprises entre 3 et 100

Chaque ligne réalisée alignement rapporte des points selon sa longueur, 
et les réactions en chaîne (combo) en apportent encore plus ! Durant la
réaction en chaine, détruire une ligne augmente le score de 
la prochaine ligne de 20%.

| Symboles | 3   | 4   | 5   | 6   | 7    | 8    | 9    | 10    | $n>10$  |
|----------|-----|-----|-----|-----|------|------|------|-------|---------|
| Score    | 300 | 450 | 650 | 900 | 1500 | 2500 | 4000 | 10000 | $2500n$ |

Lorsqu'il n'y a plus d'échanges possibles, la partie est terminée et
vous pouvez entrer dans la légende : faire partie des meilleurs scores.

### Meilleurs scores
À chaque fin de partie, vous inscrire votre nom pour faire entrer votre score
dans le classement. 

Les classements sont répartis selon la configuration de votre grille : le nombre
de symboles et la taille. Par exemple, il existe un classement pour grilles moyennes
avec 4 symboles, et un autre pour les grandes grilles avec 5 symboles. 
Chaque classement admet 12 places au maximum. 

Les meilleurs scores sont accessibles dans le menu principal, et sont sauvegardés
de manière très lisible dans le fichier `highscores.pasteque`.

### Sauvegarde
Il est possible de sauvegarder une partie en cours depuis le menu pause 
(appuyez sur P) ou en appuyant sur N. La partie sauvegardée peut être reprise
avec le bouton "Reprendre" dans le menu principal.

Une seule partie peut être sauvegardée à la fois, contenue dans le fichier
`savefile.pasteque`.

### Contrôles
* Menu principal
  * Naviguer : Flèches directionnelles, ZQSD, souris
  * Confirmer : Entrée, Espace (sauf pour les boîtes de texte), clic gauche/droit
* Jeu
  * Déplacer le curseur : Flèches directionnelles, ZQSD, souris
  * Échanger deux cases : Entrée, Espace, deux clics gauches/droits, clic glissé
  * Menu pause : P
  * Sauvegarder : N

Certains terminaux ne fonctionnent pas avec la souris. Le jeu reste intégralement
jouable sans.

## Prérequis
* CMake 3.18+ OU Make
* Git
* Windows : Visual Studio avec l'option C/C++
* Debian/Ubuntu : Le paquet `build-essential` et `libncurses-dev`
* Autres distributions Linux : juste de quoi compiler du C...

## Instructions
1. Ouvrez une fenêtre de terminal  
   **Windows uniquement** : Ouvrez "x64 Native Tools Command Prompt for VS 20xx"
2. Clonez le dépôt dans l'endroit de votre choix :
   ```sh
   git clone https://github.com/ChuechTeam/projet-pasteque.git
   ```
### Compiler avec Make
3. Entrez dans le nouveau dossier
   ```sh
   cd projet_pasteque
   ```
4. Compilez et lancez le projet avec Make :
   ```sh
   make run
   ```
### Compiler avec CMake (recommandé)
3. Créez un dossier `build` dans le dépôt cloné, et mettez-vous dans le dossier créé :
   ```sh
   cd projet-pasteque
   mkdir build
   cd build
   ```
4. Une fois dans le dossier `build`, configurez CMake à l'aide de la commande suivante :
   ```sh
   cmake ..
   ```
5. Compilez le projet avec CMake :
   ```sh
   cmake --build . -j
   ```
6. Lancez l'exécutable :  
   Linux : `./projet_pasteque`  
   Windows : `projet_pasteque.exe`