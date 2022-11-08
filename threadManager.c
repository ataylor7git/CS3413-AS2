#include "threadManager.h"
#include <pthread.h>
#include "constants.h"
#include <stdio.h>
#include "console.h"
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>

char *GAME_BOARD[] = {
"                   Score:          Lives:",
"=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-centipiede!=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"",
"",
"",
"",
"",
"",
"",
"" };

char* PLAYER_BODY[4][2] =
{
  {"|",
   "|"},
  {"|",
   "/"},
  {"|",
   "-"},
  {"|",
   "\\"}
};

char* ENEMY_L[4][1] =
{{"0~~~~~~~--"},
{"o~--~~~~~~"},
{"0~~~--~~~~"},
{"o~~~~~--~~"}};

char* ENEMY_R[4][1] =
{{"~~~~~~~--0"},
{"~--~~~~~~o"},
{"~~~--~~~~0"},
{"~~~~~--~~o"}};

char* BULLET_ANIMATE[1] =
{"|"};

//Global game running variable
bool gameRunning = true;

//Mutex variables
pthread_mutex_t playerLock;
pthread_mutex_t caterpillarLock;
pthread_mutex_t screenLock;
pthread_mutex_t runningLock;
pthread_mutex_t statsLock;

//Global Variables
int pRow = 20;
int pCol = 40;
int playerAnim = 0;
int score = 0;
int lives = 4;

//Thread Functions
void *playerFunc();
void *keyboardFunc();
void *redrawFunc();
void *caterpillarMainFunc();
void *indivCaterpillarFunc();
void *bulletMoveFunc();
void checkThread();

//Starts all of the threads and creates the mutexes
void startThreads()
{
  //Creates the mutexes
  if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD))
  {
    pthread_mutex_init(&playerLock, NULL);
    pthread_mutex_init(&screenLock, NULL);
    pthread_mutex_init(&runningLock, NULL);
    pthread_mutex_init(&caterpillarLock, NULL);
    pthread_mutex_init(&statsLock, NULL);
  }

  pthread_t threads[NUM_THREADS];

  //Player Thread
  int returnCode = pthread_create(&threads[0], NULL, playerFunc, NULL);
  checkThread(returnCode, "Player");

  //Keyboard Thread
  returnCode = pthread_create(&threads[1], NULL, keyboardFunc, NULL);
  checkThread(returnCode, "Keyboard");

  //Redraw Thread
  returnCode = pthread_create(&threads[2], NULL, redrawFunc, NULL);
  checkThread(returnCode, "Redraw");

  //Main Caterpillar Thread
  returnCode = pthread_create(&threads[3], NULL, caterpillarMainFunc, NULL);
  checkThread(returnCode, "Main Caterpillar");

  //Join Threads
  int i;
  for(i = 0; i < NUM_THREADS; i++)
  {
    returnCode = pthread_join(threads[i], NULL);
  }
  //Destroy the mutexes
  pthread_mutex_destroy(&playerLock);
  pthread_mutex_destroy(&screenLock);
  pthread_mutex_destroy(&runningLock);
  pthread_mutex_destroy(&caterpillarLock);
  pthread_mutex_destroy(&statsLock);
}

//Thread for the player animation
void *playerFunc()
{
  while(gameRunning)
  {
    char** tile = PLAYER_BODY[playerAnim];

    pthread_mutex_lock(&playerLock);
    pthread_mutex_lock(&screenLock);
    consoleClearImage(pRow, pCol, 2, 1);
    consoleDrawImage(pRow, pCol, tile, 2);
    pthread_mutex_unlock(&screenLock);
    //Iterate through the player animations
    playerAnim++;
    playerAnim = playerAnim%4;
    pthread_mutex_unlock(&playerLock);
    sleepTicks(20);

  }
  pthread_exit(NULL);
}

//Checks the keyboard Input
//Will move player, shoot bullet, and quit
void *keyboardFunc()
{
  fd_set set;
  while(gameRunning)
  {
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    struct timespec timeout = getTimeout(1); /* duration of one tick */
    int ret = pselect(FD_SETSIZE, &set, NULL, NULL, &timeout, NULL);
    if (ret >= 1)
    {
      char c = getchar();

      pthread_mutex_lock(&playerLock);
      char** tile = PLAYER_BODY[playerAnim];
      pthread_mutex_lock(&screenLock);
      consoleClearImage(pRow, pCol, 2, 1);
      pthread_mutex_unlock(&screenLock);

      if (c == MOVE_LEFT && pCol > 0) {
        pCol-= 1;
      } else if (c == MOVE_RIGHT && pCol < 79) {
        pCol+= 1;
      } else if (c == MOVE_DOWN && pRow < 22) {
        pRow+= 1;
      } else if (c == MOVE_UP && pRow > 17) {
        pRow-= 1;
      }
      //Quits the game
      else if (c == QUIT) {
        pthread_mutex_lock(&runningLock);
        gameRunning = false;
        pthread_mutex_lock(&screenLock);
        consoleFinish();
        putBanner("Game Finished");
        finalKeypress();
        pthread_mutex_unlock(&screenLock);
        pthread_mutex_unlock(&runningLock);
      }
      //Shoots from current player position
      else if(c == SHOOT)
      {
        pthread_t thread;
        int* coords = (int*)malloc((3)*sizeof(int));
        coords[0] = pRow;
        coords[1] = pCol;
        coords[2] = 1;
        int returnCode = pthread_create(&thread, NULL, bulletMoveFunc, (void*)coords);
        checkThread(returnCode, "Bullet Move");
      }
      pthread_mutex_lock(&screenLock);
      consoleDrawImage(pRow, pCol, tile, 2);
      pthread_mutex_unlock(&screenLock);
      pthread_mutex_unlock(&playerLock);
    }
  }
  pthread_exit(NULL);
}

//Redraws the screenLock
//Would also write the current score and lives
void *redrawFunc()
{
  while(gameRunning)
  {
    pthread_mutex_lock(&statsLock);
    pthread_mutex_lock(&screenLock);
    putString("000000", 0, 26, 5);
    putString("4", 0, 42, 1);
    pthread_mutex_unlock(&statsLock);
    consoleRefresh();
    pthread_mutex_unlock(&screenLock);
    sleepTicks(1);
  }
  pthread_exit(NULL);
}

//The main caterpillar function. Creates first caterpillar.
//Would also create a new caterpillar every once in a while
//Would also create a bullet at the head of a random caterpillar at random times
void *caterpillarMainFunc()
{

  pthread_t caterpillars[1];
  int returnCode = pthread_create(&caterpillars[0], NULL, indivCaterpillarFunc, NULL);
  checkThread(returnCode, "Caterpillar");
  while(gameRunning)
  {
    pthread_join(caterpillars[0], NULL);
  }
  pthread_exit(NULL);
}

//Create and move an individual caterpillar
//Will move from side to side until it gets to the bottom of the
//caterpillar part of screen.
void *indivCaterpillarFunc()
{
  int cRow = 2;
  int cCol = 80;
  int j = 0;
  int size = strlen(ENEMY_L[j][0]);
  while(gameRunning && cRow < 15)
  {
    pthread_mutex_lock(&caterpillarLock);
    pthread_mutex_lock(&screenLock);

    //Draw caterpillar going to the left
    consoleClearImage(cRow, cCol, 1, size + 5);
    consoleDrawImage(cRow, cCol, ENEMY_L[j], 1);
    //Draw caterpillar going to the right
    if(cCol < 0)
    {
      consoleClearImage(cRow + 1, abs(cCol) - size - 5, 1, size + 5);
      consoleDrawImage(cRow + 1, abs(cCol) - size, ENEMY_R[j], 1);
    }
    //Reset the column after going left and then right fully
    if(cCol < -80)
    {
      consoleClearImage(cRow + 1, abs(cCol) - size - 5, 1, size + 5);
      cCol = 80;
      cRow += 2;
    }

    pthread_mutex_unlock(&screenLock);
    //Iterate through caterpillar animation
    j++;
    j = j%4;
    //Iterate through columns
    cCol = (cCol - 1);
    pthread_mutex_unlock(&caterpillarLock);
    sleepTicks(15);
  }
  pthread_exit(NULL);
}

//Creates a bullet at specified coordinates and moves either up or down.
//Input is {row, column, up/down(1 for up, otherwise down)}
void *bulletMoveFunc(void* coords)
{
  int* coordinates = (int*) coords;

  int i = coordinates[0];
  bool continueMoving = true;
  while(continueMoving && gameRunning)
  {
    //Move upwards
    if(coordinates[2] == 1)
    {
      i--;
      continueMoving = i > 2;
    }
    //Move downwards
    else
    {
      i++;
      continueMoving = i < 24;
    }

    //Draw bullet
    pthread_mutex_lock(&screenLock);

    consoleClearImage(i, coordinates[1], 2, 1);
    consoleDrawImage(i, coordinates[1], BULLET_ANIMATE, 1);

    pthread_mutex_unlock(&screenLock);
    sleepTicks(15);
  }
  //Clean up bullet
  pthread_mutex_lock(&screenLock);
  consoleClearImage(i, coordinates[1], 1, 2);
  pthread_mutex_unlock(&screenLock);
  free(coords);
  pthread_exit(NULL);
}

//Checks the thread return code to see if there was an error
void checkThread(int returnCode, char* threadName)
{
  if(returnCode)
  {
    printf("%s thread has encountered an ERROR: %d\n", threadName, returnCode);
  }
}
