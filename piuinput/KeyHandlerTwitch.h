#ifndef _KEYHANDLER_TWITCH_H
#define _KEYHANDLER_TWITCH_H

extern char bytes_t[4];
extern char bytes_tb[2];
extern double fBPM;
extern unsigned long tlastchange;
extern unsigned long delay;
void KeyHandler_Twitch_Init(void);
void KeyHandler_Twitch_Poll(void);
void KeyHandler_Twitch_UpdateLights(char* bytes);
extern char bytes_g[4]; // Purely for graphic purposes

struct command_spec {
  char p1;
  char p2;
  double beat;
  char isHold;
  double beatEnd;
};

extern struct command_spec* comms;
extern int scomms;

unsigned long GetCurrentTime();
double GetBeat(unsigned long t);
double GetCurrentBeat();

#endif
