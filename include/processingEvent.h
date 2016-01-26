#ifndef TIME_DELAY
#define TIME_DELAY 100
#endif

#ifndef PROCESSINGEV
#define PROCESSINGEV
#include <linux/joystick.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


int convertAnalogToAngle(short analogValue);
void setNewServoAngle(int angle, FILE *fd);
void analogRecieve(unsigned int timePressed, short value, unsigned int* lastTime, FILE *servoblaster);
void listeningJoystick(int joystick, FILE *servoblaster);

#endif
