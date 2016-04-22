/*!
  @file processingEvent.c
  @date 28 January 2016

*/
#include "processingEvent.h"
/*
void setupLaser()
{
  if(wiringPiSetup()==-1)
  {
    printf("failed to open the gpio pins\n");
    exit(-1);
  }

  pinMode(0, OUTPUT); //CHOISIR LA BONNE PIN
}
*/
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

void processEvents(char eventUp, char eventDown, FILE *servoblaster, unsigned int nowTime, unsigned int* lastUpperServoMovement, int *unblockUpperServo, int defaultAmbientLight, int instantAmbiantLight, int *touchedByLaser)
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
  if(defaultAmbientLight<instantAmbiantLight)
  {
    printf("Touched by laser!\n");
  }
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

int16_t checkSoundLevel(snd_pcm_t *handle)
{
  int rc, size;
  int16_t temp2=0;
  short buffer[8*1024];
  size=sizeof(buffer)>>1;
  snd_pcm_uframes_t frames=size;
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
  temp2 = (buffer[0]<<8)+buffer[1];
  return temp2;
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

int vehicleTouched(snd_pcm_t *captureHandle)
{
  return 0;
}

void *fireThread(void *vargp)
{
  //USE WIRING PI TO USE THE PHYSICAL LASER.
  canBeFired=0;
  sleep(1);
  canBeFired=1;
  return NULL;
}

void buttonFirePressed()
{
  if(canBeFired==1)
  {
    pthread_t tid;
    pthread_create(&tid, NULL, fireThread, NULL);
    printf("SHOT FIRED\n");
  }
}

void listeningJoystick(int joystick, FILE *servoblaster, snd_pcm_t *handle)
{
  struct js_event event;
  int unblock=0, unblockUpperServo=0, ambientLight=0, touchedByLaser=0;
  __u32 lastTimeAnalog1=0, lastUpperServoMovement=0;
  __s16 eventUp=0, eventDown=0;
  canBeFired=1;

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
            buttonFirePressed();
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
    processEvents(eventUp, eventDown, servoblaster, event.time, &lastUpperServoMovement, &unblockUpperServo, ambientLight, getAmbientLight(handle, 1), &touchedByLaser);
  }
}
