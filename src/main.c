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
  //TODO initialise all the mutexes and cond
  fire=PTHREAD_MUTEX_INITIALIZER;
  fireEvent=PTHREAD_COND_INITIALIZER;

  //Initialisong the microphone levels and unmuting it. HAS TO BE SET TO THE RASPI AUDIO IN.
  pthread_t *solarArrayThreadID;

  setupLaser();
  //listeningJoystick(joystick, fd, handle);

  //Créer 5 thread:
  //  -joystick control
  //  -solar array control
  //  -turret control

  pthread_create(solarArrayThreadID, NULL, solarArrayThread, NULL);
  return 0;
}
