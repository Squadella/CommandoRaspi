/*!
  @file processingEvent.h
  @date 28 January 2016

*/
#ifndef TIME_DELAY
///TIME_DELAY is used for setting the time interval between two refreshing of the servo.
#define TIME_DELAY 100
///Define a step value.
#define STEPVALUE 1000
///Define the pin used by the laser.
#define LASERPIN 25

//First motor constants
///Define the enabling pin for motor number 1.
#define ENABLEMOTOR1 7
///Define the first entry of the first motor.
#define MOTOR1ENTRY1 0
///Define the second entry of the first motor.
#define MOTOR1ENTRY2 3

//Second motor constants
///Define the enabling pin for the second motor.
#define ENABLEMOTOR2 6
///Define the first entry of the second motor.
#define MOTOR2ENTRY1 4
///Define the second entry of the second motor.
#define MOTOR2ENTRY2 5
#endif

#ifndef PROCESSINGEV
///Define header of processingEvent.h.
#define PROCESSINGEV

///Use the newer ALSA API.
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <linux/joystick.h>
#include <alsa/asoundlib.h>
#include <wiringPi.h>

///Flag for knowing if the raspi has been hit.
int touchedByLaser;
///Flag for knowing if the raspi has ammo in his magazine.
int remainingAmmo;
///The number of time the player has been touched.
int score;
snd_pcm_t *audioHandle;


///Initialise the gpio pin for the laser.
void setupGPIOPins();

///Used to convert the value returned by the analog to an angle for the servos.
int convertAnalogToAngle(__s16 analogValue/*!<The value returned by the axis.*/);

///Used to feed the angle to the file controlling the servo.
void setNewServoAngle(int angle/*!<Angle between 60 and 250*/,
                      FILE *fd/*!<File descriptor of the servo*/,
                      char servoNumber/*!<The servo number (0 or 1)*/,
                      char modifier/*!<Modify the mode of setting the angle (relative of absolute)*/);

///Open and read the microphone.
void openMicrophone(snd_pcm_t **captureHandle/*!<The handle for accessing the microphone.*/);

void openSpeaker(snd_pcm_t **audioHandle);

void playSound(snd_pcm_t *audioHandle);

///Function for the square root of the sum of the audio buffer.
double rms(short *buffer/*!<The audio captured from the microphone.*/,
           int buffer_size/*!<The size of the buffer.*/);

///Return the max value given by the solar array during a short period of time.
int getAmbientLight(snd_pcm_t *handle/*!<The handle for accessing the microphone.*/,
                    int loopTime/*!<The lenght the system will listen to the sound (in itteration).*/);

///Thread for managing all the solar array information.
void *solarArrayThread(void *vargp);
///Mutex for thread to wait for each others.
static pthread_mutex_t initSolarArray;
///Condition for checking if the solar array thread launhed sucessfully.
int touchedCond;

///Action done by the fire thread when the button is pressed.
void *fireThread(void *vargp);

///Action done by the reload thread when the button is pressed.
void *reloadThread(void *vargp);

///Action done by the touched thread when the raspi is hit.
void *touchedThread(void *vargp);

///Thread for managing all the laser action.
void *turretThread(void *vargp);
///Mutex for thread to wait for each others.
static pthread_mutex_t initTurret;
///Mutex for the wait condition.
static pthread_mutex_t fire;
///Mutex for locking if the user if allready firing or reloading.
static pthread_mutex_t isFiring;
///Condition when the fire, reload or user has been hit.
pthread_cond_t fireEvent;
///Condition for checking if the laser launched reload.
int reloadCond;
///Condition for checking if the laser launched fire.
int fireCond;
///Flag for choosing the action after the wait condition (0=fire, 1=reload, 2=hit).
int fireValue;
///Flag for knowing the laser status.
int canBeFired;

///Thread for managing all the movement of the upper servo of the turret.
void *upperServoThread(void *vargp);
///Mutex for locking the wait condition.
static pthread_mutex_t upperServoMovement;
///Condtion send by joystickThread when the user press the appropriate button.
pthread_cond_t upperServoEvent;
///Flag for knowing the direction of the upper Servo (0=up, 1=down).
int upperServoDirection;
///The test condition for upperServoEvent.
int upperServoCond;
///The current angle of the servo.
int upperServoAngleCurrentAngle;

///Thread for managing all the movement of the lower servo of the turret.
void *lowerServoThread(void *vargp/*!<Argument containing the file descriptor for servoblaster and the value returned by the joystick.*/);
///Mutex for locking the wait condition.
static pthread_mutex_t lowerServoMovement;
///Condtion send by joystickThread when the user press the appropriate button.
pthread_cond_t lowerServoEvent;
///The test condition for lowerServoEvent.
int lowerServoCond;
///Value of the last input of the joystick.
__s16 eventValue;

///Thread for managing all the user input.
void *joystickThread(void* vargp);
///Mutex for thread to wait for each others.
static pthread_mutex_t initJoystick;

#endif
