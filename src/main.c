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

  //Initialising controler
  int joystick=open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
  if(joystick==-1)
  {
    printf("Check if the joystick is connected\n");
    exit(-1);
  }

  //Opening the file for controling the first servo
  FILE *fd;
  fd = fopen("/dev/servoblaster","w");
  if(fd==NULL)
  {
    printf("Unable to open file, servoblaster may not be installed.\n");
    exit(-1);
  }

  //Initialisong the microphone levels and unmuting it. HAS TO BE SET TO THE RASPI AUDIO IN.
  system("amixer -c 1 set Mic playback 100% unmute");

  snd_pcm_t *test;
  openMicrophone(&test);
  checkSoundLevel(test);
  //getMaxValueOfMicrophone(test);
  listeningJoystick(joystick, fd, test);
  return 0;
}
