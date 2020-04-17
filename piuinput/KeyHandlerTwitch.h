#ifndef _KEYHANDLER_TWITCH_H
#define _KEYHANDLER_TWITCH_H

extern char bytes_t[4];
extern char bytes_tb[2];
void KeyHandler_Twitch_Init(void);
void KeyHandler_Twitch_Poll(void);
void KeyHandler_Twitch_UpdateLights(char* bytes);

#endif
