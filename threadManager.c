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

char* ENEMY_BODY[4][2] =
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

pthread_mutex_t playerLock;
pthread_mutex_t screenLock;
int pRow = 20;
int pCol = 40;
int i = 0;

void *playerFunc();
void *keyboardFunc();
void *redrawFunc();
void *caterpillarMainFunc();
void *bulletMainFunc();
void checkThread();

void startThreads()
{
  if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD)) //start the game (maybe need to do this elsewhere...)
  {
    pthread_mutex_init(&playerLock, NULL);
    pthread_mutex_init(&screenLock, NULL);
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

}

void *playerFunc()
{
  //printf("Player\n");
  bool firstState = true;

  while(true)
  {

    //if(!firstState)
      //state = PLAYER_STATE_1;
    //consoleDrawImage(10, 10, state, 2);
    //sleep(1);
    char** tile = ENEMY_BODY[i];

    pthread_mutex_lock(&playerLock);
    consoleClearImage(pRow, pCol, 2, 1);
    consoleDrawImage(pRow, pCol, tile, 2);
    i++;
    i = i%4;
    pthread_mutex_unlock(&playerLock);
    consoleRefresh();
    firstState = !firstState;
    sleep(1);

  }
  pthread_exit(NULL);
}

void *keyboardFunc()
{
  fd_set set;
  while(true)
  {
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    struct timespec timeout = getTimeout(1); /* duration of one tick */
    int ret = pselect(FD_SETSIZE, &set, NULL, NULL, &timeout, NULL);
    if (ret >= 1)
    {
      char c = getchar();

      pthread_mutex_lock(&playerLock);
      char** tile = ENEMY_BODY[i];
      consoleClearImage(pRow, pCol, 2, 1);

      if (c == MOVE_LEFT) {
        pCol-= 1;
      } else if (c == MOVE_RIGHT) {
        pCol+= 1;
      } else if (c == MOVE_DOWN) {
        pRow+= 1;
      } else if (c == MOVE_UP) {
        pRow-= 1;
      }
      else if (c == QUIT) {
        pRow = pRow; //Remove
      }
      consoleDrawImage(pRow, pCol, tile, 2);
      pthread_mutex_unlock(&playerLock);
    }
  }
  pthread_exit(NULL);
}

void *redrawFunc()
{
  //printf("Redraw\n");
  while(true)
  {
    //consoleRefresh();
  }
  pthread_exit(NULL);
}

void *caterpillarMainFunc()
{
  //printf("Caterpillar\n");
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
