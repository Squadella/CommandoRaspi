/*!
  @file processingEvent.c
  @date 28 January 2016

*/
#include "processingEvent.h"

int convertAnalogToAngle(short analogValue)
{
  int angle;
  angle=310-(((344*150)+analogValue)/344);
  return angle;
}

void setNewServoAngle(int angle, FILE *servoblaster)
{
  fflush(servoblaster);
  printf("%d\n", angle);
  fprintf(servoblaster, "0=%d\n", angle);
  fflush(servoblaster);
}

void analogRecieve(unsigned int timePressed, short value, unsigned int* lastTime, FILE *servoblaster, int* unblock)
{
  if(timePressed<(*(lastTime)+TIME_DELAY))
  {
    *unblock=*unblock+1;
    if(*unblock>10)
    {
      *lastTime=0;
      *unblock=0;
    }
    return;
  }
  *lastTime=timePressed;
  setNewServoAngle(convertAnalogToAngle(value), servoblaster);
}

void listeningJoystick(int joystick, FILE *servoblaster)
{
  //short isPlaying=0;
  unsigned int lastTimeAnalog1=0/*, lastTimeAnalog2=0*/;
  struct js_event event;
  int unblock=0;
  while (1==1)
  {
    //Getting controler info
    read(joystick, &event, sizeof(event));
    switch (event.type)
    {
      case 2:
      switch (event.number)
      {
        case 3:
        analogRecieve(event.time, event.value, &lastTimeAnalog1, servoblaster, &unblock);
        break;
        case 4:
        if(event.value<-170)
        {

        }
        break;
      }
      break;
    }
  }
}
