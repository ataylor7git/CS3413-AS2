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

char* ENEMY_BODY[6][1] =
{
  {"o~~~---~~~---~~~---~~~---~~~---"},
  {"0~~---~~~---~~~---~~~---~~~---~"},
  {"o~---~~~---~~~---~~~---~~~---~~"},
  {"0---~~~---~~~---~~~---~~~---~~~"},
  {"o--~~~---~~~---~~~---~~~---~~~-"},
  {"0-~~~---~~~---~~~---~~~---~~~--"},
};

bool gameRunning = true;

pthread_mutex_t playerLock;
pthread_mutex_t caterpillarLock;
pthread_mutex_t screenLock;
pthread_mutex_t runningLock;
pthread_mutex_t statsLock;
int pRow = 20;
int pCol = 40;
int i = 0;
int score = 0;
int lives = 4;

void *playerFunc();
void *keyboardFunc();
void *redrawFunc();
void *caterpillarMainFunc();
void *indivCaterpillarFunc();
void *bulletMainFunc();
void *bulletMoveFunc();
void checkThread();

void startThreads()
{
  if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD)) //start the game (maybe need to do this elsewhere...)
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

  //Main Bullet Thread
  returnCode = pthread_create(&threads[4], NULL, bulletMainFunc, NULL);
  checkThread(returnCode, "Main Bullet");

  //Join Threads
  int i;
  for(i = 0; i < NUM_THREADS; i++)
  {
    returnCode = pthread_join(threads[i], NULL);
  }
  pthread_mutex_destroy(&playerLock);
  pthread_mutex_destroy(&screenLock);
  pthread_mutex_destroy(&runningLock);
  pthread_mutex_destroy(&caterpillarLock);
  pthread_mutex_destroy(&statsLock);
}

void *playerFunc()
{
  while(gameRunning)
  {
    char** tile = PLAYER_BODY[i];

    pthread_mutex_lock(&playerLock);
    pthread_mutex_lock(&screenLock);
    consoleClearImage(pRow, pCol, 2, 1);
    consoleDrawImage(pRow, pCol, tile, 2);
    pthread_mutex_unlock(&screenLock);
    i++;
    i = i%4;
    pthread_mutex_unlock(&playerLock);
    sleepTicks(20);

  }
  pthread_exit(NULL);
}

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
      char** tile = PLAYER_BODY[i];
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
      else if(c == SHOOT)
      {
        pthread_t thread;
        int* coords = (int*)malloc((3)*sizeof(int));
        coords[0] = pRow;
        coords[1] = pCol;
        coords[2] = 1;
        int returnCode = pthread_create(&thread, NULL, bulletMoveFunc, (void*)coords);
        checkThread(returnCode, "Bullet Move");
        //pthread_join(thread, NULL);
      }
      pthread_mutex_lock(&screenLock);
      consoleDrawImage(pRow, pCol, tile, 2);
      pthread_mutex_unlock(&screenLock);
      pthread_mutex_unlock(&playerLock);
    }
  }
  pthread_exit(NULL);
}

void *redrawFunc()
{
  //printf("Redraw\n");
  while(gameRunning)
  {
    pthread_mutex_lock(&statsLock);
    pthread_mutex_lock(&screenLock);
    putString("000000", 0, 26, 5);
    putString("4", 0, 42, 1);
    pthread_mutex_unlock(&statsLock);
    consoleRefresh();
    pthread_mutex_unlock(&screenLock);
    //pthread_mutex_lock(&statsLock);
    sleepTicks(1);

  }
  pthread_exit(NULL);
}

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

void *indivCaterpillarFunc()
{
  char* tilel[4][1] =
  {{"0~~~~~~~--"},
  {"o~--~~~~~~"},
  {"0~~~--~~~~"},
  {"o~~~~~--~~"}};

  char* tiler[4][1] =
  {{"~~~~~~~--0"},
  {"~--~~~~~~o"},
  {"~~~--~~~~0"},
  {"~~~~~--~~o"}};

  int cRow = 2;
  int cCol = 80;
  int j = 0;
  int size = strlen(tilel[j][0]);
  while(gameRunning)
  {
    pthread_mutex_lock(&caterpillarLock);
    pthread_mutex_lock(&screenLock);
    if(cRow%2 != 1)
    {
      consoleClearImage(cRow, cCol, 1, size + 5);
      consoleDrawImage(cRow, cCol, tilel[j], 1);
    }
    pthread_mutex_unlock(&screenLock);
    j++;
    j = j%4;
    cCol = (cCol - 1);
    pthread_mutex_unlock(&caterpillarLock);
    sleepTicks(15);
  }
  pthread_exit(NULL);
}

void *bulletMoveFunc(void* coords)
{
  int* coordinates = (int*) coords;
  char* bTile[1] =
  {"|"};
  int i = coordinates[0];
  bool continueMoving = true;
  while(continueMoving && gameRunning)
  {
    if(coordinates[2] == 1)
    {
      i--;
      continueMoving = i > 1;
    }
    else
    {
      i++;
      continueMoving = i < 24;
    }

    pthread_mutex_lock(&screenLock);

    consoleClearImage(i, coordinates[1], 2, 1);
    consoleDrawImage(i, coordinates[1], bTile, 1);

    pthread_mutex_unlock(&screenLock);
    sleepTicks(15);
  }
  pthread_mutex_lock(&screenLock);
  consoleClearImage(i + 1, coordinates[1], 1, 1);
  pthread_mutex_unlock(&screenLock);
  free(coords);
  pthread_exit(NULL);
}

void *bulletMainFunc()
{
  //printf("Bullet\n");
  pthread_exit(NULL);
}

void checkThread(int returnCode, char* threadName)
{
  if(returnCode)
  {
    printf("%s thread has encountered an ERROR: %d\n", threadName, returnCode);
  }
}
