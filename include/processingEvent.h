/*!
  @file processingEvent.h
  @date 28 January 2016

*/
#ifndef TIME_DELAY
///TIME_DELAY is used for setting the time interval between two refreshing of the servo.
#define TIME_DELAY 50
#define X 0
#define A 1
#define Y 2
#define B 3
#define LEFT_JOYSTICK_VERTICAL 0
#define LEFT_JOYSTICK_HORIZONTAL 1
#define RIGHT_JOYSTICK_VERTICAL 2
#define RIGHT_JOYSTICK_HORIZONTAL 3
#define HORIZONTAL 4
#define VERTICAL 5
#define START 9
#define BACK 8
#define LB 4
#define LT 6
#define RB 5
#define RT 7
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

///Used to convert the value returned by the analog to an angle for the servos.
int convertAnalogToAngle(short analogValue);
///Used to feed the angle to the file controlling the servo.
void setNewServoAngle(int angle, FILE *fd, char servoNumber, char modifier);
///Used to manage the axis input.
void analogRecieve(unsigned int timePressed, short value, unsigned int* lastTime, FILE *servoblaster, int* unblock, char servoNumber);
///Used to move the upper servo up.
void analogRecieveUpperServo(unsigned int timePressed, short value,  unsigned int* lastTime, FILE *servoblaster, int* unblock,  char servoNumber);
///Used to move down the upper servo.
void buttonRecieveDown(short value, FILE *servoblaster,  char servoNumber);
///Used to process all the entries on the joystick.
void listeningJoystick(int joystick, FILE *servoblaster);

#endif
