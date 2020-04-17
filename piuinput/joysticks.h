#ifndef _JOYSTICKS_H
#define _JOYSTICKS_H

extern char bytes_j[4];
extern char bytes_jb[2];
void init_joysticks(void);
void poll_joysticks(void);

#endif
