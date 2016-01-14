#include <linux/joystick.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "processingEvent.h"

int main()
{
  char servoNumber[10];
  strcpy(servoNumber, "echo 0=");
  char servoAngle[4];
  strcpy(servoAngle, "");
  char retourLigne[2]={'\n'};
  char path[20];
  strcpy(path, ">/dev/servoblaster");
  char servoControl[40];
  int manette=open("/dev/input/js0", O_RDONLY, O_NONBLOCK);
  int i;
  struct js_event event;
  fflush(NULL);
  while(1)
  {
    read(manette, &event, sizeof(event));
    for(i=60; i<250; i+=10)
    {
      sprintf(servoAngle, "%d", i);
      strcat(servoControl, servoNumber);
      strcat(servoControl, servoAngle);
      strcat(servoControl, path);
      strcat(servoControl, retourLigne);
      printf("%s", servoControl);
      system(servoControl);
      strcpy(servoControl, "");
      system(servoControl);
    }
  }
  return 0;
}
