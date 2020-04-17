#include "keyboards.h"

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glob.h>
#include <linux/input.h>

/* Vector of keyboards to open*/
FILE **kbds;
int i = 0, k, l;
char key_map[KEY_MAX/8+1];
char key_map2[KEY_MAX/8+1];
int nKeyboards = 0;

/* Initializes the keyboards */
void init_keyboards(void) {
  glob_t keyboards;
  glob("/dev/input/by-path/*event-kbd", 0, 0, &keyboards);

  if(keyboards.gl_pathc > 0) {
    nKeyboards = keyboards.gl_pathc;
    printf("Found %d keyboards\n", nKeyboards);
    kbds = (FILE**)malloc(nKeyboards*sizeof(FILE*));
    char **p;
    int n;
    for(p = keyboards.gl_pathv, n = 0; n < (int)(keyboards.gl_pathc); p++, n++) {
      kbds[n] = fopen(*p, "r");
      printf("Opening file: %s\n", *p);
    }
  }
  
  globfree(&keyboards);
}

#define keyp(keymap, key) (keymap[key/8] & (1 << (key % 8)))
char bytes_p[4];
char bytes_pb[2];
void poll_keyboards(void) {
  bytes_p[0] = 0xFF;
  bytes_p[1] = 0xFF;
  bytes_p[2] = 0xFF;
  bytes_p[3] = 0xFF;
  for(i = 0; i < nKeyboards; i++) if(kbds[i] != NULL)
  {
    memset(key_map, 0, sizeof(key_map));    //  Initate the array to zero's
    ioctl(fileno(kbds[i]), EVIOCGKEY(sizeof(key_map)), key_map);    //  Fill the keymap with the current keyboard state

    char byte = 0;
    if(keyp(key_map, KEY_KP7) || keyp(key_map, KEY_R))
        byte |= (0x1);
    if(keyp(key_map, KEY_KP9) || keyp(key_map, KEY_Y))
        byte |= (0x2);
    if(keyp(key_map, KEY_KP5) || keyp(key_map, KEY_G))
        byte |= (0x4);
    if(keyp(key_map, KEY_KP1) || keyp(key_map, KEY_V))
        byte |= (0x8);
    if(keyp(key_map, KEY_KP3) || keyp(key_map, KEY_N))
        byte |= (0x10);
    if(keyp(key_map, KEY_U))
        byte |= (0x20);
    if(keyp(key_map, KEY_I))
        byte |= (0x40);
    if(keyp(key_map, KEY_O))
        byte |= (0x80);
    bytes_p[2] &= ~byte;

    byte = 0;
    if(keyp(key_map, KEY_Q))
        byte |= (0x1);
    if(keyp(key_map, KEY_E))
        byte |= (0x2);
    if(keyp(key_map, KEY_S))
        byte |= (0x4);
    if(keyp(key_map, KEY_Z))
        byte |= (0x8);
    if(keyp(key_map, KEY_C))
        byte |= (0x10);
    if(keyp(key_map, KEY_P))
        byte |= (0x20);
    if(keyp(key_map, KEY_J))
        byte |= (0x40);
    if(keyp(key_map, KEY_K))
        byte |= (0x80);
    bytes_p[0] &= ~byte;

    byte = 0;
    if(keyp(key_map, KEY_L))
        byte |= (0x1);
    if(keyp(key_map, KEY_M))
        byte |= (0x2);
    if(keyp(key_map, KEY_6))
        byte |= (0x4);
    if(keyp(key_map, KEY_7))
        byte |= (0x8);
    if(keyp(key_map, KEY_8))
        byte |= (0x10);
    if(keyp(key_map, KEY_9))
        byte |= (0x20);
    if(keyp(key_map, KEY_0))
        byte |= (0x40);
    if(keyp(key_map, KEY_COMMA))
        byte |= (0x80);
    bytes_p[3] &= ~byte;

	  byte = 0;
    if(keyp(key_map, KEY_F5))
        byte |= (0x1);
    if(keyp(key_map, KEY_F7))
        byte |= (0x2);
    if(keyp(key_map, KEY_F7))
        byte |= (0x4);
    if(keyp(key_map, KEY_F8))
        byte |= (0x8);
    if(keyp(key_map, KEY_F9))
        byte |= (0x10);
    if(keyp(key_map, KEY_F10))
        byte |= (0x20);
    if(keyp(key_map, KEY_F11))
        byte |= (0x40);
    if(keyp(key_map, KEY_F12))
        byte |= (0x80);
    bytes_p[1] &= ~byte;

    if(keyp(key_map, KEY_SPACE))
    {
      bytes_p[2] &= 0xE0;
      bytes_p[0] &= 0xE0;
    }
    
    byte = 0;
    if(keyp(key_map, KEY_BACKSPACE))
        byte |= (0x1);
    if(keyp(key_map, KEY_LEFT))
        byte |= (0x2);
    if(keyp(key_map, KEY_RIGHT))
        byte |= (0x4);
    if(keyp(key_map, KEY_ENTER))
        byte |= (0x8);
    if(keyp(key_map, KEY_KPMINUS))
        byte |= (0x10);
    if(keyp(key_map, KEY_KP4))
        byte |= (0x20);
    if(keyp(key_map, KEY_KP6))
        byte |= (0x40);
    if(keyp(key_map, KEY_KPENTER))
        byte |= (0x80);
    bytes_pb[0] = ~byte;
    bytes_pb[1] = 0xFF;
  }
}
