/*!
  @file processingEvent.c
  @date 28 January 2016

*/
#include "processingEvent.h"

void setupLaser()
{
  if(wiringPiSetup()==-1)
  {
    printf("failed to open the gpio pins\n");
    exit(-1);
  }
  pinMode (25, OUTPUT);
}

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

void openMicrophone(snd_pcm_t **captureHandle)
{
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val=48000;
  snd_pcm_uframes_t frames=32;
  int dir;
  int rc;
  rc=snd_pcm_open(&handle, "hw:1", SND_PCM_STREAM_CAPTURE, 0);
  if(rc<0)
  {
    printf("Unable to open pcm device : %s\n", snd_strerror(rc));
    exit(1);
  }
  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);
  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);
  /* Set the desired hardware parameters. */
  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 1);
  /* 44100 bits/second sampling rate (CD quality) */
  snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
  /* Set period size to 32 frames. */
  snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0)
  {
    printf("unable to set hw parameters: %s\n", snd_strerror(rc));
    exit(1);
  }
  snd_pcm_hw_params_get_period_size(params, &frames, &dir);

  snd_pcm_hw_params_get_period_time(params, &val, &dir);
  *captureHandle=handle;
}

double rms(short *buffer, int buffer_size)
{
	int i;
	long int square_sum = 0.0;
	for(i=0; i<buffer_size; i++)
		square_sum += (buffer[i] * buffer[i]);

	double result = sqrt(square_sum/buffer_size);
	return result;
}

int getAmbientLight(snd_pcm_t *handle, int loopTime)
{
  int rc, size, dB;
  double tmp;
  int max=0;
  short buffer[8*1024];
  size=sizeof(buffer)>>1; //2 bytes 1 channel
  snd_pcm_uframes_t frames=size;
  while(loopTime!=0)
  {
    rc = snd_pcm_readi(handle, buffer, frames);
    if (rc == -EPIPE)
    {
      /* EPIPE means overrun */
      printf("overrun occurred\n");
      snd_pcm_prepare(handle);
    }
    else if (rc < 0)
    {
      printf("error from read: %s\n", snd_strerror(rc));
    }
    else if (rc != (int)frames)
    {
      printf("short read, read %d frames\n", rc);
    }
    tmp=rms(buffer, size);
    dB = (int)20*log10(tmp);
    if(dB>max)
    {
      max=dB;
    }
    loopTime--;
  }
  return max;
}

void *solarArrayThread(void *vargp)
{
  //Initialise the base value of the solar array.
  pthread_mutex_lock(&initSolarArray);
  int ambientLight, err;
  system("amixer -c 1 set Mic playback 100% unmute");
  snd_pcm_t *handle;
  openMicrophone(&handle);
  ambientLight=getAmbientLight(handle, 10);
  pthread_mutex_unlock(&initSolarArray);

  //Waiting for other thread to initialise
  pthread_mutex_lock(&initJoystick);
  pthread_mutex_unlock(&initJoystick);
  pthread_mutex_lock(&initTurret);
  pthread_mutex_unlock(&initTurret);

  //TODO mettre une mutex pour éviter les touches multiples.
  while(1==1)
  {
    if((ambientLight+5)<getAmbientLight(handle, 1))
    {
      //Wait for last turret action to finish
      pthread_mutex_lock(&fire);
      fireValue=2;
      pthread_mutex_unlock(&fire);
      //Launching event
      err=pthread_cond_signal(&fireEvent);
      if(err!=0)
      {
        printf("Error when setting fireEvent : %d\n", err);
        pthread_exit(NULL);
      }
      //5 seconds of invulnerability
      sleep(5);
    }
  }
  return NULL;
}

void *fireThread(void *vargp)
{
  pthread_mutex_lock(&isFiring);
  pthread_detach(pthread_self());
  printf("SHOT FIRED!\n");
  fflush(stdout);
  canBeFired=0;
  remainingAmmo--;
  digitalWrite (25, HIGH);
  sleep(1);
  digitalWrite (25, LOW);
  sleep(1);
  canBeFired=1;
  pthread_mutex_unlock(&isFiring);
  pthread_exit(NULL);
}

void *reloadThread(void *vargp)
{
  pthread_mutex_lock(&isFiring);
  pthread_detach(pthread_self());
  printf("SHOT FIRED!\n");
  canBeFired=0;
  printf("RELOADING!\n");
  sleep(2);
  remainingAmmo=5;
  canBeFired=1;
  pthread_mutex_unlock(&isFiring);
  pthread_exit(NULL);
}

void *touchedThread(void *vargp)
{
  pthread_detach(pthread_self());
  pthread_mutex_lock(&isFiring);
  touchedByLaser=1;
  score-=1;
  pthread_mutex_unlock(&isFiring);
  printf("TOUCHED PLS WAIT.");
  fflush(stdout);
  sleep(3);
  touchedByLaser=0;
  return NULL;
}

//TODO pas convaincu par le lock et unlock mutex a test
void *turretThread(void *vargp)
{
  //Initialisation phase
  pthread_mutex_lock(&initTurret);
  setupLaser();
  pthread_mutex_unlock(&initTurret);

  if(pthread_mutex_lock(&fire))
  {
    printf("Error when locking fire event");
    pthread_exit(NULL);
  }
  printf("laser has been set up.\n");

  //Wait for other thread to initialise
  pthread_mutex_lock(&initJoystick);
  pthread_mutex_unlock(&initJoystick);
  pthread_mutex_lock(&initSolarArray);
  pthread_mutex_unlock(&initSolarArray);

  while(1==1)
  {
    //Waiting for signal of others threads.
    if(pthread_cond_wait(&fireEvent, &fire))
    {
      switch (fireValue)
      {
        //The user pressed the fire button
        case 0:
          if(canBeFired==1 && touchedByLaser==0)
          {
            pthread_t tmp;
            pthread_create(&tmp, NULL, fireThread, NULL);
          }
          break;
        //The user pressed the reload button
        case 1:
          if(canBeFired==1)
          {
            pthread_t tmp2;
            pthread_create(&tmp2, NULL, reloadThread, NULL);
          }
          break;
        //The pi has been hit by the enemy
        case 2:
          ;//Empty statement for avoid a compilation error.
          pthread_t tmp3;
          pthread_create(&tmp3, NULL, touchedThread, NULL);
          break;
        //Unexpected value
        default:
          printf("Unexpected value : %d", fireValue);
          pthread_exit(NULL);
          break;
      }
    }
  }
  pthread_mutex_unlock(&fire);
  return NULL;
}

void *upperServoThread(void *vargp)
{
  FILE* servoblaster = (FILE*)vargp;
  pthread_mutex_lock(&upperServoMovement);
  while(1==1)
  {
    if(pthread_cond_wait(&upperServoEvent, &upperServoMovement))
    {
      //Going up
      if(upperServoDirection==0)
      {
        setNewServoAngle(1, servoblaster, '1', '+');
      }
      //Going down
      else
      {
        setNewServoAngle(1, servoblaster, '1', '-');
      }
      usleep(500);
    }
  }
  pthread_mutex_unlock(&upperServoMovement);
  pthread_exit(NULL);
}

void *lowerServoThread(void *vargp)
{
  pthread_mutex_lock(&lowerServoMovement);
  pthread_detach(pthread_self());
  lowerServoArg *tmp = (lowerServoArg *)vargp;
  setNewServoAngle(convertAnalogToAngle(tmp->eventValue), tmp->servoblaster, 0, ' ');
  pthread_mutex_unlock(&lowerServoMovement);
  pthread_exit(NULL);
}

void *joystickThread(void *vargp)
{
  pthread_mutex_lock(&initJoystick);
  //Initialising the controller
  int joystick=open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
  if(joystick==-1)
  {
    printf("Check if the joystick is connected\n");
    exit(-1);
  }

  //Initialising the servos
  FILE *servoblaster;
  pthread_t upperServoThreadID;
  servoblaster = fopen("/dev/servoblaster","w");
  if(servoblaster==NULL)
  {
    printf("Unable to open file, servoblaster may not be installed.\n");
    exit(-1);
  }
  //Creating the upper servo thread
  pthread_create(&upperServoThreadID, NULL, upperServoThread, (void*)servoblaster);

  //Waiting for the thread to initialise
  pthread_mutex_lock(&upperServoMovement);
  pthread_mutex_unlock(&upperServoMovement);

  //Initialising variables
  struct js_event event;
  int err;
  canBeFired=1;
  touchedByLaser=0;
  remainingAmmo=5;
  score=0;

  pthread_mutex_unlock(&initJoystick);

  //Waiting for other threads.
  pthread_mutex_lock(&initSolarArray);
  pthread_mutex_unlock(&initSolarArray);
  pthread_mutex_lock(&initTurret);
  pthread_mutex_unlock(&initTurret);

  while (1==1)
  {
    read(joystick, &event, sizeof(struct js_event));
    switch (event.type)
    {
      //Button
      case 1:
      switch (event.number)
      {
        //Button 3 : the user fire the laser
        case 2:
          if(event.value==1)
          {
            fireValue=0;
            if(remainingAmmo>0)
            {
              err=pthread_cond_broadcast(&fireEvent);
              if(err!=0)
              {
                printf("Error when setting fireEvent : %d\n", err);
                pthread_exit(NULL);
              }
            }
          }
          break;
        //Button 4 : the user reload his weapon
        case 3:
          if(event.value==1)
          {
            fireValue=1;
            err=pthread_cond_signal(&fireEvent);
            if(err!=0)
            {
              printf("Error when setting fireEvent : %d\n", err);
              pthread_exit(NULL);
            }
          }
          break;
        //Upper left trigger : Turret going up
        case 4:
          upperServoDirection=0;
          err=pthread_cond_signal(&upperServoEvent);
          if(err!=0)
          {
            printf("Error when setting upperServoEvent : %d\n", err);
            pthread_exit(NULL);
          }
          break;
        //Lower left trigger : Turret going down
        case 6:
          upperServoDirection=1;
          err=pthread_cond_signal(&upperServoEvent);
          if(err!=0)
          {
            printf("Error when setting upperServoEvent : %d\n", err);
            pthread_exit(NULL);
          }
          break;
        default:
          printf("EVENT NUMBER = %d", event.number);
          break;
      }
      break;

      //Analogic
      case 2:
      switch (event.number)
      {
        //Right axis left/right
        case 2:
          ;//Empty statement for avoid a compilation error.
          pthread_t tmp4;
          lowerServoArg* threadArg=NULL;
          threadArg->eventValue=event.value;
          threadArg->servoblaster=servoblaster;
          pthread_create(&tmp4, NULL, lowerServoThread, (void*)threadArg);
          break;
      }
      break;
    }
  }
  return NULL;
}
