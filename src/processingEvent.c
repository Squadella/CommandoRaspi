#include "processingEvent.h"

void joystickD(struct js_event event)
{

}

///Used to convert the value returned by the analog to an angle for the servos.
int convertAnalogToAngle(short analogValue)
{
  int angle;
  angle=analogValue;
  return angle;
}

///Used to feed the angle to the file controlling the servo.
void setNewServoAngle(int angle, FILE *fd)
{
  fprintf(fd, "0=%d", angle);
}
