#include "KeyHandlerTwitch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

char bytes_t[4];
char bytes_tb[2];
pthread_mutex_t mutex;
void* runserver(void*);
void HandleBuffer(int deploy_full);

void KeyHandler_Twitch_Init(void) {
  pthread_t server;
  
  if (pthread_mutex_init(&mutex, NULL) != 0) { 
    printf("\n mutex init has failed for twitch server\n"); 
    return; 
  } 

	if(pthread_create(&server, NULL, runserver, NULL) != 0){
		puts("Could not create server twitch thread");
	}
}

void error(const char *msg)
{
  perror(msg);
  exit(1);
}

#define MAXBUF (1*1024*1024)
char buf[MAXBUF];
int siz = 0;

void* runserver(void* context){

	int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(1025);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
          sizeof(serv_addr)) < 0) 
          error("ERROR on binding");
  listen(sockfd,5);
  
  int running = 1;
  while(running) {
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, 
               (struct sockaddr *) &cli_addr, 
               &clilen);
    if (newsockfd < 0) {
      error("ERROR on accept");
      continue;
    }
    
    struct timeval tv;
    gettimeofday(&tv,NULL);
    unsigned long t1 = 1000000 * tv.tv_sec + tv.tv_usec;
    while(1) {
      bzero(buffer,256);
      n = recv(newsockfd,buffer,255,MSG_DONTWAIT);
      if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
          error("ERROR reading from socket");
          break;
      }
      else {
        gettimeofday(&tv,NULL);
        unsigned long t2 = 1000000 * tv.tv_sec + tv.tv_usec;
        if((t2 - t1) > 5000000) // 5 seconds
        {
          printf("Timeout\n");
          break;
        }
      }
      
      if(n == 1 && buffer[0] == 'q') {
        printf("Quiting the server\n");
        running = 0;
        break;
      }
      
      if(buffer[0] == 'Q') {
        printf("Quiting the client\n");
        break;
      }
      
      if(n > 0) {
        // Renew the time of last connection
        gettimeofday(&tv,NULL);
        t1 = 1000000 * tv.tv_sec + tv.tv_usec;
        while((siz+n) >= MAXBUF) {
          HandleBuffer(1);
        }
        memcpy(buf + siz, buffer, n);
        siz += n;
        //printf("Here is the message (%d): %s\n", n, buffer);
        //n = write(newsockfd, "I got your message", 18);
        //if (n < 0) {
        //  error("ERROR writing to socket");
        //  break;
        //}
      }
      HandleBuffer(0);
    }
    close(newsockfd);
  }
  close(sockfd);
  return 0; 
}

void RebaseBuffer(int index) {
  // It is possible that ask us to rebase beyond the size. In this case
  // Make it empty
  if(index >= siz) {
    siz = 0;
    buf[siz] = 0;
    return;
  }
  if(index <= 0) return; // Do nothing here
  for(int k = 0, l = index; l < siz; k++, l++) {
    buf[k] = buf[l];
  }
  siz -= index;
  buf[siz] = 0;
}

int ul = 0, ur = 0, ce = 0, dl = 0, dr = 0;
void HandleBuffer(int deploy_full) {
  if(siz <= 0) return;
  //printf("Entering: HandleBuffer\n");
  buf[siz] = 0; // To treat is as string
  char* bufaux = buf;
  char* bufend = buf + siz;
  while(siz != 0) {
    char* f = strchr(bufaux, '\n');
    if(f == NULL) {
      // It ended?
      int remsiz = strlen(bufaux);
      if((bufaux+remsiz) >= bufend) {
        // Rebase at the last buffer, maybe transmission is not complete yet
        RebaseBuffer((int)(bufaux - buf));
        break;
      }
      else if(deploy_full) {
        // Drop the buffer
        siz = 0;
        buf[siz] = 0;
        printf("WARN: Buffer dropped\n");
        break;
      }
    }
    else {
      // It found the \n, convert it to \0 and do whatever
      (*f) = 0;
      //printf("Found! %s\n", bufaux);
      pthread_mutex_lock(&mutex);
      if(bufaux[0] == 'z') {
        dl = 1;
      }
      if(bufaux[0] == 'c') {
        dr = 1;
      }
      if(bufaux[0] == 's') {
        ce = 1;
      }
      if(bufaux[0] == 'q') {
        ul = 1;
      }
      if(bufaux[0] == 'e') {
        ur = 1;
      }
      pthread_mutex_unlock(&mutex);
      bufaux = f + 1;
      if(bufaux >= bufend) {
        // Just drop it, nothing else to see here
        siz = 0;
        buf[siz] = 0;
        break;
      }
    }
  }
  //printf("Current state of the buf: %s\n", buf);
}

void KeyHandler_Twitch_Poll(void) {
  pthread_mutex_lock(&mutex);
  bytes_t[0] = 0xFF;
  bytes_t[1] = 0xFF;
  bytes_t[2] = 0xFF;
  if(ul) { bytes_t[2] &= ~(0x01); ul = 0; }
  if(ur) { bytes_t[2] &= ~(0x02); ur = 0; }
  if(ce) { bytes_t[2] &= ~(0x04); ce = 0; }
  if(dl) { bytes_t[2] &= ~(0x08); dl = 0; }
  if(dr) { bytes_t[2] &= ~(0x10); dr = 0; }
  bytes_t[3] = 0xFF;
  bytes_tb[0] = 0xFF;
  bytes_tb[1] = 0xFF;
  pthread_mutex_unlock(&mutex);
}

void KeyHandler_Twitch_UpdateLights(char* bytes) {
}

