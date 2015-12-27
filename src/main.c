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
  //Initialising controler
  int joystick=open("/dev/input/js0", O_RDONLY, O_NONBLOCK);
  if(joystick==-1)
  {
    printf("Check if the joystick is connected\n");
    exit(-1);
  }

  //Methode using echo for rotating servos
  /*char servoNumber[10];
  strcpy(servoNumber, "echo 0=");
  char servoAngle[4];
  strcpy(servoAngle, "");
  char retourLigne[2]={'\n'};
  char path[20];
  strcpy(path, ">/dev/servoblaster");
  char servoControl[40];
  int i;
  fflush(NULL);
  while(1)
  {
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
  */

  //Methode using fopen and fprintf for using servos to be tested
  FILE *fd;
  fd = fopen("/dev/servoblaster","w");
  if(fd==NULL)
  {
    printf("Unable to open file, servoblaster may not be installed.\n");
    exit(-1);
  }/*
while(1)
{
  setNewServoAngle(79,fd);
  usleep(100000);
  setNewServoAngle(240,fd);
  usleep(200000);
  setNewServoAngle(79, fd);
  usleep(200000);
} */
 listeningJoystick(joystick, fd);
  return 0;
}
