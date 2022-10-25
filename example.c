
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "console.h"
#include "example.h"


#define GAME_ROWS 24
#define GAME_COLS 80

/**** DIMENSIONS MUST MATCH the ROWS/COLS */
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


#define ENEMY_HEIGHT 2
#define ENEMY_BODY_ANIM_TILES 8
char* ENEMY_BODY[ENEMY_BODY_ANIM_TILES][ENEMY_HEIGHT] =
{
  {"1",
   "1"},
  {"2",
   "2"},
  {"3",
   "3"},
  {"4",
   "4"},
  {"5",
   "5"},
  {"6",
   "6"},
  {"7",
   "7"},
  {"8",
   "8"}
};


void exampleRun()
{
  if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD)) //start the game (maybe need to do this elsewhere...)
  {
    //only run one example at once (comment others out).
    //these examples should be removed in your final submission!

    //moveEnemyExample();
    playerControlExample();
    //multipleEnemyExample();

    finalKeypress(); /* wait for final key before killing curses and game */
  }
  consoleFinish();
}


//no threading...just an example of drawing to the console
//and an example of how animation works...just drawing a picture one at a time...fast!
void moveEnemyExample(int row, int col)
{
  for (int i = 0; i<ENEMY_BODY_ANIM_TILES; i++) //loop over the whole enemy animation once
                                                //this "animation" is just the body changing appearance (changing numbers)
  {
    char** tile = ENEMY_BODY[i];

    consoleClearImage(row,col+i-1,ENEMY_HEIGHT, strlen(tile[0])); //clear the last drawing (why is this necessary?)
    consoleDrawImage(row, col+i, tile, ENEMY_HEIGHT); //draw the enemy at 10,10 but move them along once per animation image
    consoleRefresh(); //reset the state of the console drawing tool
    sleep(1); //give up our turn on the CPU
  }
}

void playerControlExample()
{
  fd_set set; /* what to check for our select call */
  bool gameRunning = true;
  int row = 5;
  int col = 30;
  char** tile = ENEMY_BODY[0];

  consoleDrawImage(row, col, tile, ENEMY_HEIGHT); //draw the enemy at 10,10 but move them along once per animation image
  consoleRefresh(); //reset the state of the console drawing tool

  while (gameRunning)
  {
    /* setup select to listen to stdin. necessary as getchar
      is blocking and cannot be easily unblocked, e.g.,
                  to end the game*/
    /* re-set each time as it can get overwritten */
    //essentially, wait for a key, or a timeout
    //read man page for pselect to understand all of this!
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    struct timespec timeout = getTimeout(1); /* duration of one tick */
    int ret = pselect(FD_SETSIZE, &set, NULL, NULL, &timeout, NULL);


    /* check if we timed out, or, event happened.
      also, game may have ended while we waited */
    if (gameRunning && ret >= 1)
    {
        char c = getchar();

        //this move code is too simple for when we get to threads...
        //there's also no error checking
        //we'll have to add in some protection...what are the critical resources here?
        consoleClearImage(row,col,ENEMY_HEIGHT, strlen(tile[0])); //clear the last drawing (why is this necessary? And why here?)
        if (c == MOVE_LEFT) {
          col-= 1;
        } else if (c == MOVE_RIGHT) {
          col+= 1;
        } else if (c == MOVE_DOWN) {
          row+= 1;
        } else if (c == MOVE_UP) {
          row-= 1;
        }
        else if (c == QUIT) {
          gameRunning = false;
        }

        //NOT THREADED
        //just draw player...using enemy from earlier
        consoleDrawImage(row, col, tile, ENEMY_HEIGHT); //draw the enemy at 10,10 but move them along once per animation image
        consoleRefresh(); //reset the state of the console drawing tool

    } /* end select bypass */

  } /* game loop*/

}

void* moveEnemyExampleT(void *v)
{
  int* pos = (int*)v;
  moveEnemyExample(pos[0], pos[1]);

  return NULL;
}


//this will break!
//why?
//try to fix this using just what we know in class! Then you'll be able to handle the assignment better
void multipleEnemyExample()
{

  pthread_t enemy1, enemy2;
  int pos1[] = {10,10};
  int pos2[] = {10,20};
  pthread_create(&enemy1, NULL, moveEnemyExampleT, (void*)pos1);
  pthread_create(&enemy2, NULL, moveEnemyExampleT, (void*)pos2);

  pthread_join(enemy1, NULL);
  pthread_join(enemy2, NULL);
}
