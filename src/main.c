#include <linux/joystick.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include "processingEvent.h"

int main()
{
  //TODO initialise all the mutexes and cond and global vars
  //fire=PTHREAD_MUTEX_INITIALIZER;
  //fireEvent=PTHREAD_COND_INITIALIZER;

  //Initialising main threads
  pthread_t solarArrayThreadID;
  pthread_t joystickThreadID;
  pthread_t turretThreadID;

  pthread_create(&solarArrayThreadID, NULL, solarArrayThread, NULL);
  pthread_create(&joystickThreadID, NULL, joystickThread, NULL);
  pthread_create(&turretThreadID, NULL, turretThread, NULL);

  pthread_join(solarArrayThreadID, NULL);
  pthread_join(joystickThreadID, NULL);
  pthread_join(turretThreadID, NULL);
  return 0;
}
