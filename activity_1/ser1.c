#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>

#define MAXPENDING 5    /* Maximum number of simultaneous connections */
#define BUFFSIZE 1024      /* Size of message to be reeived */
#define DEFAULTPORT 8888

void err_sys(char *mess) { perror(mess); exit(1); }


int main(int argc, char *argv[]) {
  struct sockaddr_in echoserver, echoclient;
  int serversock, clientsock;
  int port = DEFAULTPORT;
  /* Check input arguments */
  if (argc > 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  if (argc == 2){
    if(!check_port(argv[1])){
        printf("Invalid port number: %s\n", argv[1]);
        exit(1);
    }
    port = atoi(argv[1]);
  }
  
  /*Seed*/
  srand(time(NULL));

  /* Create TCP socket */
  serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (serversock < 0) {
    err_sys("Error socket");
  }

  /* Set information for sockaddr_in structure */
  memset(&echoserver, 0, sizeof(echoserver));       /* Reset memory */
  echoserver.sin_family = AF_INET;                  /* Internet/IP */
  echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any address */
  echoserver.sin_port = htons(port);       /* Server port */

  /* Bind socket */
  if (bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0) {
    err_sys("Error bind");
  }

  /* Listen socket */
  if (listen(serversock, MAXPENDING) < 0) {
    err_sys("Error listen");
  }

  /* As a server we are in an infinite loop, waiting forever */
  while (1) {
    unsigned int clientlen = sizeof(echoclient);

    /* Wait for a connection from a client */
    clientsock = accept(serversock, (struct sockaddr *) &echoclient, &clientlen);
    if (clientsock < 0) {
      err_sys("Error accept");
    }
    fprintf(stdout, "Client: %s\n", inet_ntoa(echoclient.sin_addr));

    /* Call function to handle socket */
    handle_client(clientsock);
  }
}

int check_port(char *port_src){
    
    int port = atoi(port_src);
    return port > 0 && port < 65536;
}

void handle_client(int sock) {
  char buffer[BUFFSIZE];
  int received = -1;
  int guess, num_guesses = 0;
  int answer = rand()%100;
  /* Just wait */
  while (1){
    // Receive guess from client
        int len = recv(sock, buffer, BUFFSIZE, 0);
        if (len < 0) {
            perror("recv");
            break;
        } else if (len == 0) {
            printf("Connection closed by client\n");
            break;
        }

        buffer[len] = '\0';
        guess = atoi(buffer);
        num_guesses++;

        // Compare guess to answer and send result to client
        if (guess < answer) {
            if (send(sock, "More bro\n", 9, 0) < 0) {
                perror("send");
                break;
            }
        } else if (guess > answer) {
            if (send(sock, "Less dude\n", 10, 0) < 0) {
                perror("send");
                break;
            }
        } else {
            if (send(sock, "There you go\n", 13, 0) < 0) {
                perror("send");
                break;
            }
            printf("Client guessed %d in %d tries\n", guess, num_guesses);
            break;
        }
  }

  /* Close socket */
  close(sock);
}