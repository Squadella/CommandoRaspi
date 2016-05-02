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

int convertAnalogToAngle(__s16 analogValue)
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
      fireValue=2;
      //Launching event
      while(touchedCond==0)
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
  pthread_detach(pthread_self());
  pthread_mutex_lock(&isFiring);
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
  return NULL;
}

void *reloadThread(void *vargp)
{
  pthread_detach(pthread_self());
  pthread_mutex_lock(&isFiring);
  canBeFired=0;
  printf("RELOADING!\n");
  sleep(2);
  remainingAmmo=5;
  canBeFired=1;
  pthread_mutex_unlock(&isFiring);
  return NULL;
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
  upperServoAngleCurrentAngle=250;
  pthread_mutex_unlock(&initTurret);


  //Wait for other thread to initialise
  pthread_mutex_lock(&initJoystick);
  pthread_mutex_unlock(&initJoystick);
  pthread_mutex_lock(&initSolarArray);
  pthread_mutex_unlock(&initSolarArray);
  if(pthread_mutex_lock(&fire))
  {
    printf("Error when locking fire event");
    pthread_exit(NULL);
  }
  printf("laser has been set up.\n");
  while(1==1)
  {
    //Waiting for signal of others threads.
    //TODO add the cond when the pi is hit.
    pthread_cond_wait(&fireEvent, &fire);
    switch (fireValue)
    {
      //The user pressed the fire button
      case 0:
        fireCond=0;
        if(canBeFired==1 && touchedByLaser==0)
        {
          pthread_t tmp;
          printf("Button fire pressed.\n");
          pthread_create(&tmp, NULL, fireThread, NULL);
        }
        break;
      //The user pressed the reload button
      case 1:
      reloadCond=1;
        if(canBeFired==1)
        {
          pthread_t tmp2;
          printf("Button reload pressed.\n");
          pthread_create(&tmp2, NULL, reloadThread, NULL);
        }
        break;
      //The pi has been hit by the enemy
      case 2:
        touchedCond=1;
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
  pthread_mutex_unlock(&fire);
  return NULL;
}

void *upperServoThread(void *vargp)
{
  pthread_detach(pthread_self());
  FILE* servoblaster = (FILE*)vargp;

  while(1==1)
  {
    pthread_mutex_lock(&upperServoMovement);
    pthread_cond_wait(&upperServoEvent, &upperServoMovement);
    while(1==1)
    {
      printf("Going down NOW!\n");
      upperServoCond=1;
      //Going up
      if(upperServoDirection==0)
      {
        if(upperServoAngleCurrentAngle<250)
          upperServoAngleCurrentAngle++;
        setNewServoAngle(upperServoAngleCurrentAngle, servoblaster, '1', ' ');
      }
      //Going down
      else if(upperServoDirection==1)
      {
        if(upperServoAngleCurrentAngle>130)
          upperServoAngleCurrentAngle--;
        setNewServoAngle(upperServoAngleCurrentAngle, servoblaster, '1', ' ');
      }
      else if(upperServoDirection==3)
      {
        break;
      }
      usleep(5000);
    }
    printf("le deuxième while.\n");
    pthread_mutex_unlock(&upperServoMovement);
  }
  pthread_exit(NULL);
}

void *lowerServoThread(void *vargp)
{
  pthread_detach(pthread_self());
  FILE* servoblaster = (FILE*)vargp;
  pthread_mutex_lock(&lowerServoMovement);
  setNewServoAngle(convertAnalogToAngle(eventValue), servoblaster, '0', ' ');
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
  servoblaster = fopen("/dev/servoblaster","w");
  if(servoblaster==NULL)
  {
    printf("Unable to open file, servoblaster may not be installed.\n");
    exit(-1);
  }
  pthread_t upperServoThreadID, lowerServoThreadID;
  pthread_create(&upperServoThreadID, NULL, upperServoThread, (void*)servoblaster);

  //Waiting for the thread to initialise
  pthread_mutex_lock(&upperServoMovement);
  pthread_mutex_unlock(&upperServoMovement);

  //Initialising variables
  struct js_event event;
  int err, upperServoAllreadyMoving=0;
  __u32 lastTimePressedFire=0, lastTimePressedReload=0;
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

  event.value=0;

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
          if(event.value==1 && lastTimePressedFire+1000<event.time)
          {
            fireValue=0;
            if(remainingAmmo>0)
            {
              lastTimePressedFire=event.time;
              fireCond=1;
              err=pthread_cond_signal(&fireEvent);
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
          if(event.value==1 && lastTimePressedReload+1000<event.time)
          {
            reloadCond=0;
            fireValue=1;
            while(reloadCond==0)
              err=pthread_cond_signal(&fireEvent);

            if(err!=0)
            {
              printf("Error when setting fireEvent : %d\n", err);
              pthread_exit(NULL);
            }
          }
          break;
        //Upper left trigger : Turret going down
        case 4:
          if(event.value==1 && upperServoAllreadyMoving==0)
          {
            printf("Servo start going down\n");
            upperServoAllreadyMoving=1;
            upperServoDirection=1;
            upperServoCond=0;
            while(upperServoCond==0)
            {
              pthread_cond_signal(&upperServoEvent);
            }
            printf("Signaled\n");
          }
          else if(event.value==0 && upperServoAllreadyMoving==1)
          {
            printf("Stoped.\n");
            upperServoAllreadyMoving=0;
            upperServoDirection=3;
          }
          break;
        //Lower left trigger : Turret going up
        case 6:
          if(event.value==1 && upperServoAllreadyMoving==0)
          {
            printf("Servo start going down\n");
            upperServoAllreadyMoving=1;
            upperServoDirection=0;
            upperServoCond=0;
            while(upperServoCond==0)
            {
              pthread_cond_signal(&upperServoEvent);
            }
            printf("Signaled\n");
          }
          else if(event.value==0 && upperServoAllreadyMoving==1)
          {
            printf("Stoped.\n");
            upperServoAllreadyMoving=0;
            upperServoDirection=3;
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
          ;
          eventValue=event.value;
          pthread_create(&lowerServoThreadID, NULL, lowerServoThread, (void*)servoblaster);
          break;
      }
      break;
    }
  }
  return NULL;
}
