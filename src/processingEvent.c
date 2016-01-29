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

void analogRecieveUpperServo(unsigned int timePressed, short value,  unsigned int* lastTime, FILE *servoblaster, int* unblock,  char servoNumber)
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
        //Left joystick (up and down)
        case LEFT_JOYSTICK_VERTICAL:
        break;

        //Left joystick (right and left)
        case LEFT_JOYSTICK_HORIZONTAL:
        break;

        //Right joystick (up and down)
        case RIGHT_JOYSTICK_VERTICAL:
        analogRecieve(event.time, event.value, &lastTimeAnalog1, servoblaster, &unblock, '0');
        break;

        //Left joystick (right and left)
        case RIGHT_JOYSTICK_HORIZONTAL:

        break;

        //Vertical direction
        case VERTICAL:
        break;

        //Horizontal direction
        case HORIZONTAL:
        break;
      }
    }
    else
    {
      switch (event.number)
      {
        //The X button
        case X:
        break;

        //The A button
        case A:
        break;

        //The Y button
        case Y:
        break;

        //The B button
        case B:
        break;

        //Left button
        case LB:
        analogRecieveUpperServo(event.time, event.value,  &lastTimeAnalog2, servoblaster, &unblock2,  '1');
        break;

        //Right button
        case RB:
        break;

        //Left trigger
        case LT:
        buttonRecieveDown(event.value, servoblaster, '1');
        break;

        //Right trigger
        case RT:
        break;

        //Start button
        case START:
        break;

        //Back button
        case BACK:
        break;
      }
    }
  }
}
