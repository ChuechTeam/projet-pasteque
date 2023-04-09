# Projet Pasteque
Le projet d'info

## Prérequis
* CMake 3.18+
* Git
* Windows : Visual Studio avec l'option C/C++
* Debian/Ubuntu : Le paquet `build-essential`
* Autres distributions Linux : juste de quoi compiler du C...

## Instructions pour compiler
1. Ouvrez une fenêtre de terminal  
   **Windows uniquement** : Ouvrez "x64 Native Tools Command Prompt for VS 20xx"
2. Clonez le dépôt dans l'endroit de votre choix :
   ```sh
   git clone https://github.com/ChuechTeam/projet-pasteque.git
   ```
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