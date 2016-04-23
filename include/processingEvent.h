/*!
  @file processingEvent.h
  @date 28 January 2016

*/
#ifndef TIME_DELAY
///TIME_DELAY is used for setting the time interval between two refreshing of the servo.
#define TIME_DELAY 100
///Define a step value.
#define STEPVALUE 1000
///Define the id of the X button on the controller.
#define X 0
///Define the id of the A button on the controller.
#define A 1
///Define the id of the Y button on the controller.
#define Y 2
///Define the id of the B button on the controller.
#define B 3
///Define the id of the left axis in vertical.
#define LEFT_JOYSTICK_VERTICAL 0
///Define the id of the left axis in horizontal.
#define LEFT_JOYSTICK_HORIZONTAL 1
///Define the id of the right axis in vertical.
#define RIGHT_JOYSTICK_VERTICAL 2
///Define the id of the right axis in horizontal.
#define RIGHT_JOYSTICK_HORIZONTAL 3
///Define the id of the horizontal arrows.
#define HORIZONTAL 4
///Define the id of the vertical arrows.
#define VERTICAL 5
///Define the id of the start button.
#define START 9
///Define the id of the back button.
#define BACK 8
///Define the id of the left button.
#define LB 4
///Define the id of the left trigger.
#define LT 6
///Define the id of the right button.
#define RB 5
///Define the id of the right trigger.
#define RT 7
#endif

#ifndef PROCESSINGEV
///Define header of processingEvent.h.
#define PROCESSINGEV

///Use the newer ALSA API.
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <linux/joystick.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <pthread.h>
#include <wiringPi.h>

///Flag for knowing the laser status.
int canBeFired;
pthread_mutex_t fire;
pthread_cond_t fireEvent;
int fireValue;
///Flag for knowing if the raspi has been hit.
int touchedByLaser;
///Flag for knowing if the raspi has ammo in his magazine.
int remainingAmmo;
///Flag for knowing if the raspi is reloading.
int isReloading;

int score;


///Initialise the gpio pin for the laser.
void setupLaser();

///Used to convert the value returned by the analog to an angle for the servos.
int convertAnalogToAngle(short analogValue/*!<The value returned by the axis.*/);

///Used to feed the angle to the file controlling the servo.
void setNewServoAngle(int angle/*!<Angle between 60 and 250*/,
                      FILE *fd/*!<File descriptor of the servo*/,
                      char servoNumber/*!<The servo number (0 or 1)*/,
                      char modifier/*!<Modify the mode of setting the angle (relative of absolute)*/);

///Used to manage the axis input.
void analogRecieve(unsigned int timePressed/*!<The time in ms when the key has been pressed.*/,
                   short value/*!<The value returned by the button.*/,
                   unsigned int* lastTime/*!<The last time same button has been pressed.*/,
                   FILE *servoblaster/*!<The file descriptor of the servoblaster file.*/,
                   int* unblock/*!<Value for unstucking the loop.*/,
                   char servoNumber/*!<The servo number to activate (0 or 1)*/);

///Action done by the touched thread when the raspi is hit.
void *touchedThread(void *vargp);

///Processing all events at the end of the while.
void processEvents(char eventUp/*!<Flag that allows us to tell if the user is pressing the key to aim higher with the laser.*/,
                   char eventDown/*!<Flag that allows us to tell if the user is pressing the key to aim lower with the laser.*/,
                   FILE *servoblaster/*!<The file descriptor of the servoblaster file.*/,
                   unsigned int nowTime/*!<Time when the button has been pressed.*/,
                   unsigned int* lastUpperServoMovement/*!<The last time same button has been pressed.*/,
                   int *unblockUpperServo/*!<Value for unstucking the loop.*/,
                   int defaultAmbientLight/*!<Value of the ambient light at the begining of the program.*/,
                   int instantAmbiantLight/*!<The actual value of the microphone.*/,
                   pthread_t tid);

///Open and read the microphone.
void openMicrophone(snd_pcm_t **captureHandle/*!<The handle for accessing the microphone.*/);

///Function for the square root of the sum of the audio buffer.
double rms(short *buffer/*!<The audio captured from the microphone.*/,
           int buffer_size/*!<The size of the buffer.*/);

///Test function for getting the maximum value given by the microphone.
void getMaxValueOfMicrophone(snd_pcm_t *handle/*!<The handle for accessing the microphone.*/);

///Return the max value given by the solar array during a short period of time.
int getAmbientLight(snd_pcm_t *handle/*!<The handle for accessing the microphone.*/,
                    int loopTime/*!<The lenght the system will listen to the sound (in itteration).*/);

///Action done by the fire thread when the button is pressed.
void *fireThread();

///Launch the fire thread.
void buttonFirePressed(pthread_t *tid/*!<The id of the thread. This variable is needed by other functions.*/);

///Action done by the reload thread when the button is pressed.
void *reloadThread(void *vargp/*!<The id of the fire thread, used to wait for it.*/);

///Launch the reload thread.
void reloadButtonPressed(pthread_t tid/*!<The id of the fire thread, used to wait for it.*/);

///Used to process all the entries on the joystick.
void listeningJoystick(int joystick/*!<File descriptor of the joystick device file.*/,
                       FILE *servoblaster/*!<File descriptor of the servoblaster pseudo file.*/,
                       snd_pcm_t *handle/*!<The handle used to read the microphone input.*/);

///Mutex for thread to wait for each others
pthread_mutex_t initSolarArray;
///Thread for managing all the solar array information.
void *solarArrayThread(void *vargp);

///Mutex for thread to wait for each others
pthread_mutex_t initTurret;
///Thread for managing all the laser action.
void *turretThread(void *vargp);

///Mutex for thread to wait for each others
pthread_mutex_t initJoystick;
///Thread for managing all the user input.
void *joystickThread(void* vargp);

#endif
