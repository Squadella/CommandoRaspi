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

void openMicrophone(snd_pcm_t *captureHandle)
{
  snd_pcm_hw_params_t *hwParams;
  unsigned int rate = 44100;
  if((snd_pcm_open(&captureHandle, "hw:0", SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK))<0)
  {
    printf("Fail opening the microphone.\n");
    exit(-1);
  }
  if(snd_pcm_hw_params_malloc(&hwParams)<0)
  {
    printf("Can't allocate parameters structure.\n");
    exit(-1);
  }
  if(snd_pcm_hw_params_any(captureHandle, hwParams)<0)
  {
    printf("Can't configure hardware.\n");
    exit(-1);
  }
  if(snd_pcm_hw_params_set_access(captureHandle, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED)<0)
  {
    printf("Can't set the access type.\n");
    exit(-1);
  }
  if(snd_pcm_hw_params_set_format(captureHandle, hwParams, SND_PCM_FORMAT_S16_LE)<0)
  {
    printf("Can't set the capture format.\n");
    exit(-1);
  }
  if(snd_pcm_hw_params_set_rate_near(captureHandle, hwParams, &rate, 0)<0)
  {
    printf("Can't set the sample rate.\n");
    exit(-1);
  }
  if(snd_pcm_hw_params_set_channels(captureHandle, hwParams, 1)<0)
  {
    printf("Can't set the channel number\n");
    exit(-1);
  }
  if(snd_pcm_hw_params(captureHandle, hwParams)<0)
  {
    printf("Failed to apply parameters.\n");
    exit(-1);
  }
  snd_pcm_hw_params_free(hwParams);
  if(snd_pcm_prepare(captureHandle)<0)
  {
    printf("Can't prepare the audio interface.\n");
    exit(-1);
  }
}

int16_t checkSoundLevel(snd_pcm_t *captureHandle)
{
  int16_t buf[2];
  snd_pcm_readi(captureHandle, buf, 2);
  return buf[0];
}

int vehicleTouched(snd_pcm_t *captureHandle)
{
  if(abs(checkSoundLevel(captureHandle))>STEPVALUE)
  {
    return 1;
  }
  return 0;
}

void listeningJoystick(int joystick, FILE *servoblaster)
{
  //short isPlaying=0;
  snd_pcm_t *captureHandle;
  unsigned int lastUpperServoMovement=0;
  int unblockUpperServo=0;
  unsigned int lastTimeAnalog1=0;
  struct js_event event;
  int unblock=0;
  char servoUp=0, servoDown=0;
  openMicrophone(captureHandle);
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
    if(vehicleTouched(captureHandle)==1)
    {
      printf("touched!\n");
    }
  }
}
