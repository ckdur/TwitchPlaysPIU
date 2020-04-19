#include "autoplay.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// PRIME v21 address for autoplay
int auto_available = 0;
int auto_1 = -1;
int auto_2 = -1;
unsigned char* demo_var = (unsigned char*)0xE35CDE0;
unsigned int* player1_auto = (unsigned int*)0xAB7E678;
unsigned int* player2_auto = (unsigned int*)0xAB9F8A8;

void check_autoplay(const char* title) {
  if(strstr(title, "PRIME")) {
    printf("The autoplay is supported\n");
    auto_available = 1;
  }
}

void update_autoplay () {
  
  if(!auto_available) return;
  
	if(auto_1 != -1) // HEH DEMO PLAY HUEHUE
    (*player1_auto) = auto_1;
	else
    (*player1_auto) = 0xFFFFFFFF;
  
	if(auto_2 != -1) // HEH DEMO PLAY HUEHUE
    (*player2_auto) = auto_2;
	else
    (*player2_auto) = 0xFFFFFFFF;
}
