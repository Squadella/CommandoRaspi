/*!
  @file processingEvent.c
  @date 28 January 2016

*/
#include "processingEvent.h"

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

void processEvents(char eventUp, char eventDown, FILE *servoblaster, unsigned int nowTime, unsigned int* lastUpperServoMovement, int *unblockUpperServo)
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
}

int openMicrophone()
{
  FILE *fd;
  fd = fopen("tourbicouille","w");
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val=44100;
  snd_pcm_uframes_t frames=32;
  int dir;
  int rc;
  int16_t temp2;
  char *buffer;
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
  snd_pcm_hw_params_set_channels(handle, params, 2);
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

  size=frames*4; //2 bytes 1 channel
  buffer=(char*)malloc(size);

  snd_pcm_hw_params_get_period_time(params, &val, &dir);
  int loop=1000;
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
    temp2 = (buffer[0]<<8)+buffer[1];
    fprintf(fd, "%d;", temp2);
    loop--;
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);
  return 0;
}

int16_t checkSoundLevel(snd_pcm_t *captureHandle)
{
  return 0;
}

int vehicleTouched(snd_pcm_t *captureHandle)
{
  return 0;
}

void listeningJoystick(int joystick, FILE *servoblaster)
{
  unsigned int lastUpperServoMovement=0;
  int unblockUpperServo=0;
  unsigned int lastTimeAnalog1=0;
  struct js_event event;
  int unblock=0;
  char servoUp=0, servoDown=0;
  while (1==1)
  {
    //Getting controler info
    read(joystick, &event, sizeof(event));
    if(event.type==2)
    {
      switch (event.number)
      {
        //Left joystick (up and down)
        case LEFT_JOYSTICK_VERTICAL:
        break;

        //Left joystick (right and left)
        case LEFT_JOYSTICK_HORIZONTAL:
        break;

        //Right joystick (up and down)
        case RIGHT_JOYSTICK_VERTICAL:
        analogRecieve(event.time, event.value, &lastTimeAnalog1, servoblaster, &unblock, '0');
        break;

        //Left joystick (right and left)
        case RIGHT_JOYSTICK_HORIZONTAL:

        break;

        //Vertical direction
        case VERTICAL:
        break;

        //Horizontal direction
        case HORIZONTAL:
        break;
      }
    }
    else
    {
      switch (event.number)
      {
        //The X button
        case X:
        break;

        //The A button
        case A:
        break;

        //The Y button
        case Y:
        break;

        //The B button
        case B:
        break;

        //Left button
        case LB:
        servoUp=event.value;
        break;

        //Right button
        case RB:
        break;

        //Left trigger
        case LT:
        servoDown=event.value;
        break;

        //Right trigger
        case RT:
        break;

        //Start button
        case START:
        break;

        //Back button
        case BACK:
        break;
      }
    }
    processEvents(servoUp, servoDown, servoblaster, event.time, &lastUpperServoMovement, &unblockUpperServo);
  }
}
