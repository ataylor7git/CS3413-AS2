#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <pthread.h>

//will probably want a file just for game globals. There should be a lot!
#define MOVE_LEFT 'a'
#define MOVE_RIGHT 'd'
#define MOVE_UP 'w'
#define MOVE_DOWN 's'
#define SHOOT ' '
#define QUIT 'q'

void exampleRun();
void moveEnemyExample();
void playerControlExample();
void multipleEnemyExample();
void* moveEnemyExampleT(void *v);

#endif
