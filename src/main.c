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
  /*
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
  */
  openMicrophone();
  //listeningJoystick(joystick, fd);
  return 0;
}
