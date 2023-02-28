#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define DEFAULTPORT 8888
#define DEFAULTIP "127.0.0.1"
#define BUFFSIZE 1024

void err_sys(char *mess) { perror(mess); exit(1); }

int main(int argc, char *argv[]) {
  struct sockaddr_in echoserver;
  char buffer[BUFFSIZE];
  char message[BUFFSIZE];
  unsigned int echolen;
  int sock;
  int received = 0;
  int port = DEFAULTPORT;
  char *ip = DEFAULTIP;
  /* Check input arguments */
  if (argc > 3) {
    fprintf(stderr, "Usage: %s <port> <ip_adress>\n", argv[0]);
    exit(1);
  }
  else if (argc > 1) {
    /* The first argument is the port */
    port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
      fprintf(stderr, "Invalid port number: %s\n", argv[1]);
      exit(EXIT_FAILURE);
    }
    /* If we have a second argument, it's the IP address */
    if (argc > 2) {
      ip = argv[2];
    }
  }

  /* Try to create TCP socket */
  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0) {
    err_sys("Error socket");
  }

  /* Set information for sockaddr_in */
  memset(&echoserver, 0, sizeof(echoserver));       /* Reset memory */
  echoserver.sin_family = AF_INET;                  /* Internet/IP */
  echoserver.sin_addr.s_addr = inet_addr(ip);  /* Server address */
  echoserver.sin_port = htons(port);       /* Server port */

  /* Try to have a connection with the server */
  if (connect(sock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0) {
    err_sys("Error connect");
  }

  /* Just wait */
  while (1){
    printf("Enter your Guess, player... Remember that it's a number between 0 and 100 \n");
    fgets(message, 5, stdin);
    
    if (send(sock, message, strlen(message), 0) < 0) {
        err_sys("Error send");
    }

    if ((received = recv(sock, buffer, BUFFSIZE-1, 0)) == -1) {
            err_sys("Error receive");
    }
    buffer[received] = '\0';
    printf(buffer);
    if(received>10) break;
  }

  /* Close socket */
  close(sock);

  exit(0);
}