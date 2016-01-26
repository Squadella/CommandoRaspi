#include "processingEvent.h"

///Used to convert the value returned by the analog to an angle for the servos.
int convertAnalogToAngle(short analogValue)
{
  int angle;
  angle=((344*150)+analogValue)/344;
  return angle;
}

///Used to feed the angle to the file controlling the servo.
void setNewServoAngle(int angle, FILE *servoblaster)
{
  fprintf(servoblaster, "0=%d", angle);
}

void analogRecieve(unsigned int timePressed, short value, unsigned int* lastTime, FILE *servoblaster)
{
  if(timePressed<*lastTime+TIME_DELAY)
  {
    return;
  }
  *lastTime=timePressed;
  setNewServoAngle(convertAnalogToAngle(value), servoblaster);
}

///Used to process all the entries on the joystick
void listeningJoystick(int joystick, FILE *servoblaster)
{
  short isPlaying=0;
  unsigned int lastTimeAnalog1=0/*, lastTimeAnalog2=0*/;
  struct js_event event;

  while (isPlaying)
  {
    //Getting controler info
    read(joystick, &event, sizeof(event));
    switch (event.type)
    {
      case 2:
      switch (event.number)
      {
        case 3:
        analogRecieve(event.time, event.value, &lastTimeAnalog1, servoblaster);
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
