#include "joysticks.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#define JOY_DEV "/dev/input/js0"

/* THE FOLLOWING IS THE BUTTON-DESCRIPTION*/
uint32_t lst_mrk[] =
{0x00000400, 0x00000000, 0x00000004, 0x00000000, 0x00000001,
 0x00000002, 0x00000008, 0x00000010, 0x00000000, 0x00000000,
 0x00000000, 0x00000000, 0x00000000, 0x00000000};
 uint32_t lst_mrkb[] =
{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x00000000, 0x00000000, 0x00000000};

int size_mrk = sizeof(lst_mrk)/sizeof(uint32_t);
int size_mrkb = sizeof(lst_mrkb)/sizeof(uint32_t);

int joy_fd, *axis=NULL, num_of_axis=0, num_of_buttons=0, x;
char *button=NULL, name_of_joystick[80];
struct js_event js;

void init_joysticks(void) {
  if( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 )
	{
		printf("CKDUR: Couldn't open joystick\n" );
	}
	else
	{
    ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
    ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
    ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );
    axis = (int *) calloc( num_of_axis, sizeof( int ) );
    button = (char *) calloc( num_of_buttons, sizeof( char ) );
    printf("CKDUR: Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n"
        , name_of_joystick
        , num_of_axis
        , num_of_buttons );
    fcntl( joy_fd, F_SETFL, O_NONBLOCK );	/* use non-blocking mode */
	}
}

char bytes_j[4];
char bytes_jb[2];
uint32_t mem = 0xFFFFFFFF;
uint32_t memb = 0xFFFFFFFF;
void poll_joysticks() {
  if(joy_fd != -1)//else
  {
    //////if(hFile != NULL) fprintf(hFile, "KUKI: Read Joy\n");
    while(read(joy_fd, &js, sizeof(struct js_event)) > 0)
    {
      if((js.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON) {
        if(js.number < size_mrk)
        {
          if(js.value)
            mem &= ~lst_mrk[js.number];
          else
            mem |=  lst_mrk[js.number];
        }
        if(js.number < size_mrkb)
        {
          if(js.value)
            memb &= ~lst_mrkb[js.number];
          else
            memb |=  lst_mrkb[js.number];
        }
      }
      

    }
  }
  bytes_j[0] = (mem & 0xFF);
  bytes_j[1] = ((mem >> 8) & 0xFF);
  bytes_j[2] = ((mem >> 16) & 0xFF);
  bytes_j[3] = ((mem >> 24) & 0xFF);
  bytes_jb[0] = (memb & 0xFF);
  bytes_jb[1] = ((memb >> 8) & 0xFF);
}
