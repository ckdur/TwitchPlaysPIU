#define _GNU_SOURCE

// gcc -g test_server.c -o test_server -lpthread

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

void* runserver(void*);

int main(){
	pthread_t server;

	if(pthread_create(&server, NULL, runserver, NULL) != 0){
		puts("Could not create server thread");

		return EXIT_FAILURE;
	}

	pthread_join(server, NULL);

	return EXIT_SUCCESS;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

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
        printf("Here is the message (%d): %s\n", n, buffer);
        n = write(newsockfd, "I got your message", 18);
        if (n < 0) {
          error("ERROR writing to socket");
          break;
        }
      }
    }
    close(newsockfd);
  }
  close(sockfd);
  return 0; 
}

