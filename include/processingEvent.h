#ifndef PROCESSINGEV
#define PROCESSINGEV
#include <linux/joystick.h>
#include <stdio.h>

void joystickD(struct js_event event);
int convertAnalogToAngle(short analogValue);
void setNewServoAngle(int angle, FILE *fd);

#endif
