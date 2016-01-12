#include <linux/joystick.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "processingEvent.h"

int main()
{
  char servoNumber[3]={'0', '='};
  char servoAngle[4];
  char servoControl[7];
  int manette=open("/dev/input/js0", O_RDONLY, O_NONBLOCK);
  int servo1=open("/dev/servoblaster", O_WRONLY,O_NONBLOCK);
  int i;
  struct js_event event;
  short valueOfButton;
  unsigned char eventType;
  unsigned char axisNumber;
  while(1)
  {
    read(manette, &event, sizeof(event));
    valueOfButton=event.value;
    eventType=event.type;
    axisNumber=event.number;
    printf("%d, %d, %d\n", valueOfButton, eventType, axisNumber);
    for(i=60; i<250; i++)
    {
      sprintf(servoAngle, "%d", i);
      strcat(servoControl, servoNumber);
      strcat(servoControl, servoAngle);
      write(servo1, servoControl, 100);
      printf("%s", servoControl);
      memset(servoControl,0,sizeof(servoControl));
      printf("%s\n", servoControl);
    }
    /*switch (event.type)
    {
      case 2:
        switch (event.number)
        {
          case 3:
            joystickD(event);
          case 4:
            if(event.value<-170)
            {
              joystickD(event);
            }
        }
    }*/
  }
  return 0;
}
