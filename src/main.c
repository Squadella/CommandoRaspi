#include "processingEvent.h"

int main()
{
  //TODO initialise all the mutexes and cond and global vars
  //Mutex initialisation
  pthread_mutex_init(&initSolarArray, NULL);
  pthread_mutex_init(&initTurret, NULL);
  pthread_mutex_init(&fire, NULL);
  pthread_mutex_init(&isFiring, NULL);
  pthread_mutex_init(&upperServoMovement, NULL);
  pthread_mutex_init(&lowerServoMovement, NULL);
  pthread_mutex_init(&initJoystick, NULL);

  //Condtion initialisation
  pthread_cond_init(&fireEvent, NULL);
  pthread_cond_init(&upperServoEvent, NULL);
  pthread_cond_init(&lowerServoEvent, NULL);

  //Initialising the GPIO pins
  setupGPIOPins();
  openSpeaker(&audioHandle);

  //Initialising main threads
  pthread_t solarArrayThreadID;
  pthread_t joystickThreadID;
  pthread_t turretThreadID;

  pthread_create(&solarArrayThreadID, NULL, solarArrayThread, NULL);
  pthread_create(&joystickThreadID, NULL, joystickThread, NULL);
  pthread_create(&turretThreadID, NULL, turretThread, NULL);

  pthread_join(solarArrayThreadID, NULL);
  pthread_join(joystickThreadID, NULL);
  pthread_join(turretThreadID, NULL);
  return 0;
}
