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

void setNewServoAngle(int angle, FILE *servoblaster, char servoNumber, char modifier)
{
  fflush(servoblaster);
  printf("%d\n", angle);
  fprintf(servoblaster, "%c=%c%d\n", servoNumber, modifier, angle);
  fflush(servoblaster);
}

void analogRecieve(unsigned int timePressed, short value, unsigned int* lastTime, FILE *servoblaster, int* unblock,  char servoNumber)
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
  setNewServoAngle(convertAnalogToAngle(value), servoblaster, servoNumber, ' ');
}

void analogRecieveUp(unsigned int timePressed, short value,  unsigned int* lastTime, FILE *servoblaster, int* unblock,  char servoNumber)
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
  if(value<0)
  {
    return;
  }
  setNewServoAngle(1, servoblaster, servoNumber, '+');
}

void buttonRecieveDown(short value, FILE *servoblaster,  char servoNumber)
{
  if(value!=1)
  {
    return;
  }
  setNewServoAngle(1, servoblaster, servoNumber, '-');
}

void listeningJoystick(int joystick, FILE *servoblaster)
{
  //short isPlaying=0;
  unsigned int lastTimeAnalog1=0, lastTimeAnalog2=0;
  struct js_event event;
  int unblock=0, unblock2=0;
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
        analogRecieveUp(event.time, event.value,  &lastTimeAnalog2, servoblaster, &unblock2,  '1');
        break;

        //Left joystick controlling the laser turret (Up and down direction)
        case 3:
        analogRecieve(event.time, event.value, &lastTimeAnalog1, servoblaster, &unblock, '0');
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
        buttonRecieveDown(event.value, servoblaster, '1');
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
