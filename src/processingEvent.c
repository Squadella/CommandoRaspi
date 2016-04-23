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


void analogRecieve(unsigned int timePressed, short value, unsigned int* lastTime, FILE *servoblaster, int* unblock,  char servoNumber)
{
  if(timePressed<(*(lastTime)+TIME_DELAY))
  {
    *unblock=*unblock+1;
    if(*unblock>10)
    {
      *lastTime=0;
      *unblock=0;
    }
    return;
  }
  *lastTime=timePressed;
  setNewServoAngle(convertAnalogToAngle(value), servoblaster, servoNumber, ' ');
}

void processEvents(char eventUp, char eventDown, FILE *servoblaster, unsigned int nowTime, unsigned int* lastUpperServoMovement, int *unblockUpperServo, int defaultAmbientLight, int instantAmbiantLight, pthread_t tid)
{
  if(nowTime<(*(lastUpperServoMovement)+TIME_DELAY))
  {
    *unblockUpperServo=*unblockUpperServo+1;
    if(*unblockUpperServo>10)
    {
      *lastUpperServoMovement=0;
      *unblockUpperServo=0;
    }
    return;
  }
  if(((*lastUpperServoMovement)+TIME_DELAY) > nowTime)
  {
    return;
  }
  if(eventUp && eventDown)
  {
    return;
  }
  if(eventUp)
  {
    setNewServoAngle(1, servoblaster, '1', '+');
    *lastUpperServoMovement=nowTime;
  }
  else if(eventDown)
  {
    setNewServoAngle(1, servoblaster, '1', '-');
    *lastUpperServoMovement=nowTime;
  }
  if((defaultAmbientLight+5)<instantAmbiantLight && (touchedByLaser==0))
  {
    pthread_t tid2;
    pthread_create(&tid2, NULL, touchedThread, (void*)tid);
  }
}

void *touchedThread(void *vargp)
{
  pthread_t tid=(pthread_t)vargp;
  touchedByLaser=1;
  if(canBeFired==0)
  {
    pthread_join(tid, NULL);
  }
  canBeFired=0;
  score-=1;
  printf("TOUCHED PLS WAIT.");
  fflush(stdout);
  sleep(3);
  canBeFired=1;
  touchedByLaser=0;
  return NULL;
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

void getMaxValueOfMicrophone(snd_pcm_t *handle)
{
  int loop=1000, rc, size, dB;
  double tmp;
  int max=0;
  short buffer[8*1024];
  size=sizeof(buffer)>>1; //2 bytes 1 channel
  snd_pcm_uframes_t frames=size;
  loop=1000;
  while(loop!=0)
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
    printf("tmp2 = %d\n", buffer[0]);
    tmp=rms(buffer, size);
    dB = (int)20*log10(tmp);
    printf("%d\n", dB);
    if(dB>max)
    {
      printf("%d max dB\n", dB);
      max=dB;
    }
    fflush(stdout);
    loop--;
  }
  printf("max = %d\n", max);
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

void *fireThread(void *vargp)
{
  //USE WIRING PI TO USE THE PHYSICAL LASER.
  canBeFired=0;
  remainingAmmo--;
  digitalWrite (25, HIGH);
  sleep(1);
  digitalWrite (25, LOW);
  sleep(1);
  canBeFired=1;
  return NULL;
}

void buttonFirePressed(pthread_t *tid)
{
  if(canBeFired==1 && remainingAmmo!=0)
  {
    pthread_create(tid, NULL, fireThread, NULL);
    printf("SHOT FIRED\n");
    printf("%d\n", score);
  }
}

void *reloadThread(void *vargp)
{
  pthread_t tid = (pthread_t)vargp;
  isReloading=1;
  if(canBeFired==0)
  {
    pthread_join(tid, NULL);
  }
  printf("RELOADING!\n");
  canBeFired=0;
  sleep(2);
  remainingAmmo=5;
  canBeFired=1;
  isReloading=0;
  return NULL;
}

void reloadButtonPressed(pthread_t tid)
{
  if(isReloading==0)
  {
    pthread_t tid2;
    pthread_create(&tid2, NULL, reloadThread, (void*)tid);
  }
}


void listeningJoystick(int joystick, FILE *servoblaster, snd_pcm_t *handle)
{

  struct js_event event;
  int unblock=0, unblockUpperServo=0, ambientLight=0;
  __u32 lastTimeAnalog1=0, lastUpperServoMovement=0;
  __s16 eventUp=0, eventDown=0;
  canBeFired=1;
  touchedByLaser=0;
  remainingAmmo=5;
  isReloading=0;
  score=0;

  pthread_t tid;

  ambientLight=getAmbientLight(handle, 10);
  printf("%d\n", ambientLight);
  while(1==1)
  {
    read(joystick, &event, sizeof(struct js_event));
    switch (event.type)
    {
      //Button
      case 1:
      switch (event.number)
      {
        //Button 3
        case 2:
          if(event.value==1)
          {
            buttonFirePressed(&tid);
          }
          break;
        //Button 4
        case 3:
          if(event.value==1)
          {
            reloadButtonPressed(tid);
          }
          break;
        //Upper left trigger
        case 4:
          eventUp=event.value;
          break;
        //Lower left trigger
        case 6:
          eventDown=event.value;
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
          analogRecieve(event.time, event.value, &lastTimeAnalog1, servoblaster, &unblock, '0');
          break;
        //Right axis up/down
        case 3:

          break;
      }
      break;
    }
    processEvents(eventUp, eventDown, servoblaster, event.time, &lastUpperServoMovement, &unblockUpperServo, ambientLight, getAmbientLight(handle, 1), tid);
  }
}

void *solarArrayThread(void *vargp)
{
  //Initialise the base value of the solar array.
  int ambientLight;
  system("amixer -c 1 set Mic playback 100% unmute");
  snd_pcm_t *handle;
  openMicrophone(&handle);
  ambientLight=getAmbientLight(handle, 10);

  //TODO mettre une mutex pour éviter les touches multiples.
  int err;
  while(1==1)
  {
    if((ambientLight+5)<getAmbientLight(handle, 1))
    {
      pthread_mutex_lock(&fire);
      fireValue=2;
      pthread_mutex_unlock(&fire);
      err=pthread_cond_signal(&fireEvent);
      if(err!=0)
      {
        printf("Error when setting fireEvent : %d\n", err);
        pthread_exit(NULL);
      }
      sleep(5);
    }
  }
  return NULL;
}

void *turretThread(void *vargp)
{
  while(1==1)
  {
    if(pthread_mutex_lock(&fire))
    {
      printf("Error when locking fire event");
      pthread_exit(NULL);
    }
    //Waiting for signal of others threads.
    if(pthread_cond_wait(&fireEvent, &fire))
    {
      switch (fireValue)
      {
        //The user pressed the fire button
        case 0:
          break;
        //The user pressed the reload button
        case 1:
          break;
        //The pi has been hit by the enemy
        case 2:
          break;
        //Unexpected value
        default:
          printf("Unexpected value : %d", fireValue);
          pthread_exit(NULL);
          break;
      }
    }
  }
  return NULL;
}

void *joystickThread(void* vargp)
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

  //Initialising variables
  struct js_event event;
  int unblock=0, err /*, unblockUpperServo=0*/;
  __u32 lastTimeAnalog1=0/*, lastUpperServoMovement=0*/;
  //__s16 eventUp=0, eventDown=0;
  canBeFired=1;
  touchedByLaser=0;
  remainingAmmo=5;
  isReloading=0;
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
            err=pthread_cond_signal(&fireEvent);
            if(err!=0)
            {
              printf("Error when setting fireEvent : %d\n", err);
              pthread_exit(NULL);
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
          //TODO faire des threads détaché qui font monter de 1 la tourelle (temporisation a faire)
          break;
        //Lower left trigger : Turret going down
        case 6:
          //TODO faire des threads détaché qui font descendre de 1 la tourelle (temporisation a faire)
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
          analogRecieve(event.time, event.value, &lastTimeAnalog1, servoblaster, &unblock, '0');
          break;
        //Right axis up/down
        case 3:

          break;
      }
      break;
    }
  }
  return NULL;
}
