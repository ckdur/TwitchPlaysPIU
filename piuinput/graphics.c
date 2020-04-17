#include "graphics.h"
#include "piuinput_twitch.h"
#include "KeyHandlerTwitch.h"
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

Display *d;
Window w;
XEvent e;
int s;

void* runhere(void* context) {
  GC gc;
  Colormap screen_colormap;
  XColor red, brown, blue, yellow, green, white;
  Status rc;
  
  gc = DefaultGC(d, s);
  screen_colormap = DefaultColormap(d, s);
  
  rc = XAllocNamedColor(d, screen_colormap, "red", &red, &red);
  if (rc == 0) {
    fprintf(stderr, "XAllocNamedColor - failed to allocated 'red' color.\n");
    exit(1);
  }
  rc = XAllocNamedColor(d, screen_colormap, "brown", &brown, &brown);
  if (rc == 0) {
    fprintf(stderr, "XAllocNamedColor - failed to allocated 'brown' color.\n");
    exit(1);
  }
  rc = XAllocNamedColor(d, screen_colormap, "blue", &blue, &blue);
  if (rc == 0) {
    fprintf(stderr, "XAllocNamedColor - failed to allocated 'blue' color.\n");
    exit(1);
  }
  rc = XAllocNamedColor(d, screen_colormap, "yellow", &yellow, &yellow);
  if (rc == 0) {
    fprintf(stderr, "XAllocNamedColor - failed to allocated 'yellow' color.\n");
    exit(1);
  }
  rc = XAllocNamedColor(d, screen_colormap, "green", &green, &green);
  if (rc == 0) {
    fprintf(stderr, "XAllocNamedColor - failed to allocated 'green' color.\n");
    exit(1);
  }
  rc = XAllocNamedColor(d, screen_colormap, "white", &white, &white);
  if (rc == 0) {
    fprintf(stderr, "XAllocNamedColor - failed to allocated 'white' color.\n");
    exit(1);
  }
  struct timeval tv;
  gettimeofday(&tv,NULL);
  unsigned long t1 = 1000000 * tv.tv_sec + tv.tv_usec;
  while (1) {
    if (XPending(d) > 0)
    {
      XNextEvent(d, &e);
      if (e.type == Expose) {
        
        if(!(bytes_f[0] & 0x1)) {
          XSetForeground(d, gc, red.pixel);
          XFillRectangle(d, w, gc, 5, 5, 60, 80);
        }
        if(!(bytes_f[0] & 0x2)) {
          XSetForeground(d, gc, red.pixel);
          XFillRectangle(d, w, gc, 5+60*2, 5, 60, 80);
        }
        if(!(bytes_f[0] & 0x4)) {
          XSetForeground(d, gc, yellow.pixel);
          XFillRectangle(d, w, gc, 5+60, 5+70, 60, 60);
        }
        if(!(bytes_f[0] & 0x8)) {
          XSetForeground(d, gc, blue.pixel);
          XFillRectangle(d, w, gc, 5, 5+70+50, 60, 80);
        }
        if(!(bytes_f[0] & 0x10)) {
          XSetForeground(d, gc, blue.pixel);
          XFillRectangle(d, w, gc, 5+60*2, 5+70+50, 60, 80);
        }
        
        if(!(bytes_f[2] & 0x1)) {
          XSetForeground(d, gc, red.pixel);
          XFillRectangle(d, w, gc, 60*3+5+5, 5, 60, 80);
        }
        if(!(bytes_f[2] & 0x2)) {
          XSetForeground(d, gc, red.pixel);
          XFillRectangle(d, w, gc, 60*3+5+5+60*2, 5, 60, 80);
        }
        if(!(bytes_f[2] & 0x4)) {
          XSetForeground(d, gc, yellow.pixel);
          XFillRectangle(d, w, gc, 60*3+5+5+60, 5+70, 60, 60);
        }
        if(!(bytes_f[2] & 0x8)) {
          XSetForeground(d, gc, blue.pixel);
          XFillRectangle(d, w, gc, 60*3+5+5, 5+70+50, 60, 80);
        }
        if(!(bytes_f[2] & 0x10)) {
          XSetForeground(d, gc, blue.pixel);
          XFillRectangle(d, w, gc, 60*3+5+5+60*2, 5+70+50, 60, 80);
        }
        
        XSetForeground(d, gc, white.pixel);
        XDrawRectangle(d, w, gc, 5, 5, 60, 80);
        XDrawRectangle(d, w, gc, 5+60*2, 5, 60, 80);
        XDrawRectangle(d, w, gc, 5+60, 5+70, 60, 60);
        XDrawRectangle(d, w, gc, 5, 5+70+50, 60, 80);
        XDrawRectangle(d, w, gc, 5+60*2, 5+70+50, 60, 80);
        XDrawRectangle(d, w, gc, 60*3+5+5, 5, 60, 80);
        XDrawRectangle(d, w, gc, 60*3+5+5+60*2, 5, 60, 80);
        XDrawRectangle(d, w, gc, 60*3+5+5+60, 5+70, 60, 60);
        XDrawRectangle(d, w, gc, 60*3+5+5, 5+70+50, 60, 80);
        XDrawRectangle(d, w, gc, 60*3+5+5+60*2, 5+70+50, 60, 80);
        
        for(int i = 0; i < 6; i++) {
          XDrawLine(d, w, gc, 375+i*32, 5, 375+i*32, 5+192);
        }
        
        for(int i = 0; i < 7; i++) {
          XDrawLine(d, w, gc, 375, 5+i*32, 535, 5+i*32);
        }
        
        if(bytes_l[3] & 0x04) { // L2 Clone
          XSetForeground(d, gc, red.pixel);
          XFillRectangle(d, w, gc, 375, 197, 32, 10);
        }
        if(bytes_l[2] & 0x80) { // L1
          XSetForeground(d, gc, green.pixel);
          XFillRectangle(d, w, gc, 375+32, 197, 32, 10);
        }
        if(bytes_l[1] & 0x04) { // Neon
          XSetForeground(d, gc, blue.pixel);
          XFillRectangle(d, w, gc, 375+32*2, 197, 32, 10);
        }
        if(bytes_l[3] & 0x01) { // L1 Clone
          XSetForeground(d, gc, green.pixel);
          XFillRectangle(d, w, gc, 375+32*3, 197, 32, 10);
        }
        if(bytes_l[3] & 0x02) { // L2
          XSetForeground(d, gc, red.pixel);
          XFillRectangle(d, w, gc, 375+32*4, 197, 32, 10);
        }
        
        // Lights are:
        // Neon:      bytes_l[1] & 0x04
        // L1:        bytes_l[2] & 0x80
        // L1(clone): bytes_l[3] & 0x01
        // L2:        bytes_l[3] & 0x02
        // L2(clone): bytes_l[3] & 0x04
        
        #define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
        #define BYTE_TO_BINARY(byte)  \
          (byte & 0x80 ? '1' : '0'), \
          (byte & 0x40 ? '1' : '0'), \
          (byte & 0x20 ? '1' : '0'), \
          (byte & 0x10 ? '1' : '0'), \
          (byte & 0x08 ? '1' : '0'), \
          (byte & 0x04 ? '1' : '0'), \
          (byte & 0x02 ? '1' : '0'), \
          (byte & 0x01 ? '1' : '0') 
        //char msg[255];
        //sprintf(msg, BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN, 
        //  BYTE_TO_BINARY(bytes_l[3]), 
        //  BYTE_TO_BINARY(bytes_l[2]), 
        //  BYTE_TO_BINARY(bytes_l[1]), 
        //  BYTE_TO_BINARY(bytes_l[0]));
        // From x=375 available until 535
        //XDrawString(d, w, gc, 70, 5+70+50+30, msg, strlen(msg));
        //printf("Drawn: %.2x%.2x\n", (unsigned int)bytes_f[2] & 0xff, (unsigned int)bytes_f[0] & 0xff);
      }
    }
    gettimeofday(&tv,NULL);
    unsigned long t2 = 1000000 * tv.tv_sec + tv.tv_usec;
    if((t2 - t1) > 10000) {
      t1 = t2;
      XClearArea(d, w, 0, 0, 540, 210, True);
    }
  }
}


void init_graphics(void) {

  d = XOpenDisplay(NULL);
  if (d == NULL) {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }

  s = DefaultScreen(d);
  w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 540, 210, 0,
                         BlackPixel(d, s), BlackPixel(d, s));
  XStoreName(d, w, "PIUIOMap");
  XSelectInput(d, w, ExposureMask | KeyPressMask);
  XMapWindow(d, w);

  pthread_t th;

  if(pthread_create(&th, NULL, runhere, NULL) != 0){
    puts("Could not create graphics thread");
  }
}

void update_graphics(void) {
}

