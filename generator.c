#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "simulator.h"

void main() {
  // Set up the random seed
  srand(time(NULL));

  int ppid = getpid();
  char command[40];

  while(1) {
    for (int i=0; i<5; i++) {
      if (getpid() == ppid) { // if parent, fork another process
        fork();
      }

      // Start off with a random location and direction
      short x = (int)(rand()/(double)(RAND_MAX)*CITY_WIDTH);
      short y = (int)(rand()/(double)(RAND_MAX)*CITY_HEIGHT);
      short direction = (int)((rand()/(double)(RAND_MAX))*360 - 180);

      if (getpid() != ppid) { // if child, execute vehicle process
        sprintf(command, "./vehicle %d %d %d &",x,y,direction);
        system(command);
        return;
      }

    }
    sleep(1);
  }
}
