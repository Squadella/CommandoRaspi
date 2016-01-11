#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>

int main()
{
  int manette=open("/dev/input/js0", O_RDONLY, O_NONBLOCK);
  struct js_event event;
  short valueOfButton;
  unsigned char eventType;
  unsigned char axisNumber;
  while(1)
  {
    read(manette, &event, sizeof(event));
    valueOfButton=event.value;
    eventType=event.type;
    axisNumber=event.number;
    printf("%d, %d, %d\n", valueOfButton, eventType, axisNumber);
  }
  return 0;
}
