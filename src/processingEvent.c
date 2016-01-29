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
    if(event.type==2)
    {
      switch (event.number)
      {
        //Right joystick (up and down)
        case 0:
        break;

        //Right joystick (right and left)
        case 1:
        break;

        //Left trigger
        case 2:
        break;

        //Left joystick controlling the laser turret (Up and down direction)
        case 3:
        analogRecieve(event.time, event.value, &lastTimeAnalog1, servoblaster, &unblock);
        break;

        //Left joystick (right and left)
        case 4:
        break;

        //Right trigger
        case 5:
        break;

        //Horizontal axis (left and right)
        case 6:
        break;

        //Vertical axis (up and down)
        case 7:
        break;
      }
    }
    else
    {
      switch (event.number)
      {
        //The A button
        case 0:
        break;

        //The B button
        case 1:
        break;

        //The X button
        case 2:
        break;

        //The Y button
        case 3:
        break;

        //Left button
        case 4:
        break;

        //Right button
        case 5:
        break;

        //Back button
        case 6:
        break;

        //Start button
        case 7:
        break;
      }
    }
  }
}
