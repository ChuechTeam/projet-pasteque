#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "libGameRGR2.h"
#include "local.h"

GameData* createGame(int        nbCharX,
                     int        nbCharY,
                     void*      pUserData,
                     Callbacks* pCb,
                     int        fps){
    // Local vars
    Screen*   pScreen = NULL;
    GameData* pGame   = NULL;
    
    // Check parameters
    if(nbCharX <= 0 || nbCharY <= 0){
        RAGE_QUIT(100, "Bad game dimensions : nbCharX=%d / nbCharY=%d\n", nbCharX, nbCharY);
    }
    // Check parameter pointers
    if(pUserData == NULL){
        RAGE_QUIT(110, "Empty user data pointer !\n");
    }
    if(pCb == NULL){
        RAGE_QUIT(120, "Empty callback pointer !\n");
    }
    // Check callbacks
    if( pCb->cbInit == NULL ){
        RAGE_QUIT(130, "Empty INIT callback pointer !\n");
    }        
    if( pCb->cbEvent == NULL ){
        RAGE_QUIT(131, "Empty EVENT callback pointer !\n");
    }                
    if( pCb->cbUpdate == NULL ){
        RAGE_QUIT(132, "Empty UPDATE callback pointer !\n");
    }        
    if( pCb->cbDraw == NULL ){
        RAGE_QUIT(133, "Empty DRAW callback pointer !\n");
    }        
    if( pCb->cbFinish == NULL ){
        RAGE_QUIT(134, "Empty FINISH callback pointer !\n");    
    }        
    // Allocate SCREEN structure
    pScreen = malloc( sizeof(Screen) );
    if(pScreen == NULL){
        RAGE_QUIT(140, "Screen malloc failed !\n");    
    }
    // Allocate Game struct
    pGame = malloc( sizeof(GameData) );
    if( pGame==NULL ){
        RAGE_QUIT(150, "Screen malloc failed !\n");    
    }
    // Fill game structure
    pScreen->width        = nbCharX;
    pScreen->height       = nbCharY;
    pGame->pScreen        = pScreen;
    pGame->pUserData      = pUserData;
    pGame->pUserCallbacks = pCb;
    pGame->displayFPS     = fps;
#if _WIN32
    // PASTEQUE MOD: Set up the timer for 60 FPS sync (Windows only)
    pGame->pWinTimer = CreateWaitableTimerEx(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
#endif
    // return game structure
    return pGame;  
}

void gameLoop(GameData* pGame){
    // Locals
    WINDOW*    pWin = NULL;
    Screen*    pScr = NULL;
    void*      pDat = NULL;
    Callbacks* pCb  = NULL;
    int       loop = 0;
    unsigned long startTime = 0;
    unsigned long endTime   = 0;
    unsigned long frameTime = 0;
    // Check params
    checkGame(pGame, 200);
    // Store pointers locally
    pScr = pGame->pScreen;
    pDat = pGame->pUserData;
    pCb  = pGame->pUserCallbacks;
    
    // Locales
    setlocale(LC_ALL, "");
        
    // Init of GFX (curses library)
    pWin = initscr();
    start_color();
    noecho();
    curs_set(0);
    nodelay(pWin, TRUE);
    // PASTEQUE MOD: Mouse support
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
    // Enable all mouse events for using XTerm control sequences.
    // This solution works for some terminals, but not all of them.
    // TODO: Override the TERM env with a modded terminfo with both 256color and 1003.
    // https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-Mouse-Tracking
    printf("\033[?1003h");
    fflush(stdout);
#if USE_CURSES_KEYPAD
    keypad(stdscr, TRUE);
#endif
    init_pair(1, 7, 0);   

    // Call init here
    pCb->cbInit(pDat, pScr);
    
    // First time measurement
    startTime = getTimeMicros();  
      
    //=======================================
    // START of game loop
    //=======================================
    while(loop==0){
        
        //--------------------------
        // (call event if needed)
        //--------------------------
        loop |= checkEvent(pGame);
        
        //--------------------------
        // (call update)
        //--------------------------
        loop |= pCb->cbUpdate(pDat, pScr, frameTime);
        
        //--------------------------
        // (call draw + display FPS)
        //--------------------------
        // Clear must not be called to avoid flickering
        // BUT the user MUST ensure to redraw the whole part of the screen
        //clear();
        pCb->cbDraw(pDat, pScr);
        if(pGame->displayFPS != 0){
            move(0,0);
            attron(COLOR_PAIR(1)); // PASTEQUE MOD: Stop FPS flickering
            printw("|FPS %ld|", (unsigned long)(0.99 + 1000000.0/frameTime));
        }
        refresh();

        //--------------------------
        // Wait for next frame
        //--------------------------
        // Get frame time
        endTime   = getTimeMicros();
        frameTime = endTime - startTime;
        startTime = endTime; 
        // Wait to achieve 60FPS
        if(frameTime <= 16666){
#ifdef _WIN32
            // PASTEQUE MOD: Wait using the Windows timer API to mimic usleep
            LARGE_INTEGER waitTime;
            long long duration = (16666-frameTime)*10; // Micro -> 100*Nano = 10*Micro
            // apparently the time has to be NEGATIVE, so it is considered as
            // a relative offset
            waitTime.QuadPart = -duration;
            SetWaitableTimer(pGame->pWinTimer, &waitTime, 0, NULL, NULL, FALSE);
            WaitForSingleObject(pGame->pWinTimer, INFINITE);
#else
            usleep(16666-frameTime);
#endif
        }       
    }
    //=======================================
    // END of game loop
    //=======================================

    //--------------------------
    // Call finish here and exit
    //--------------------------
    pCb->cbFinish(pDat);

    // Finish curses
    echo();
    endwin();

#if _WIN32
    // PASTEQUE MOD: Clear the sync timer for Windows
    CloseHandle(pGame->pWinTimer);
#endif
}



