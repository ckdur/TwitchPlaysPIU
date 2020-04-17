
// PRIME v21 address for autoplay
int auto_available = 0;
int auto_1 = -1;
int auto_2 = -1;
unsigned char* demo_var = (unsigned char*)0xE35CDE0;
unsigned int* player1_auto = (unsigned int*)0xAB7E678;
unsigned int* player2_auto = (unsigned int*)0xAB9F8A8;

void update_autoplay () {
  
  if(!auto_available) return;
  
	if(auto_1 != -1) // HEH DEMO PLAY HUEHUE
    (*player1_auto) = auto_1;
	else
    (*player1_auto) = 0xFFFFFFFF;
  
	if(auto_2 != -1) // HEH DEMO PLAY HUEHUE
    (*player2_auto) = auto_1;
	else
    (*player2_auto) = 0xFFFFFFFF;
}
