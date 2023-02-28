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
#define MAX_LINE_LENGTH 256
#define MAX_LINES 1024

void err_sys(char *mess) { perror(mess); exit(1); }


int main(int argc, char *argv[]) {
    struct sockaddr_in tcp_server, tcp_client;
    struct sockaddr_in udp_server, udp_client;
    unsigned int echolen, clientlen;
    int udp_sock;
    int serversock, clientsock;
    int num_lines = 0;

    /* Check input arguments */
    if (argc != 4 ) {
        fprintf(stderr, "Usage: %s <port_udp> <ip> <port_tcp>\n", argv[0]);
        exit(1);
    }
    
    /* Create UDP socket */
    udp_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_sock < 0) {
        err_sys("Error on socket creation");
    }
    /* Configure/set socket address for the server */
    memset(&udp_server, 0, sizeof(udp_server));       /* Erase the memory area */
    udp_server.sin_family = AF_INET;                  /* Internet/IP */
    udp_server.sin_addr.s_addr = inet_addr(argv[2]);  /* IP address */
    udp_server.sin_port = htons(atoi(argv[1]));       /* Server port */

    /* Get number of lines*/
    echolen = strlen(argv[2]);
    if (sendto(udp_sock, argv[2], echolen, 0, (struct sockaddr *) &udp_server, sizeof(udp_server)) != echolen) {
        err_sys("error writing word on socket");
    }
    clientlen = sizeof(udp_client);
    if (recvfrom(udp_sock, &num_lines, sizeof(int), 0, (struct sockaddr *) &udp_client, &clientlen) != sizeof(int)) {
        err_sys("Error reading");
    }

    /*Seed*/
    srand(time(NULL));
    
    /* Create TCP socket */
    serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serversock < 0) {
        err_sys("Error socket");
    }

    /* Set information for sockaddr_in structure */
    memset(&tcp_server, 0, sizeof(tcp_server));       /* Reset memory */
    tcp_server.sin_family = AF_INET;                  /* Internet/IP */
    tcp_server.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any address */
    tcp_server.sin_port = htons(atoi(argv[3]));       /* Server port */

    /* Bind socket */
    if (bind(serversock, (struct sockaddr *) &tcp_server, sizeof(tcp_server)) < 0) {
        err_sys("Error bind");
    }

    /* Listen socket */
    if (listen(serversock, MAXPENDING) < 0) {
        err_sys("Error listen");
    }

    /* As a server we are in an infinite loop, waiting forever */
    while (1) {
        clientlen = sizeof(tcp_client);

        /* Wait for a connection from a client */
        clientsock = accept(serversock, (struct sockaddr *) &tcp_client, &clientlen);
        if (clientsock < 0) {
        err_sys("Error accept");
        }
        fprintf(stdout, "Client: %s\n", inet_ntoa(tcp_client.sin_addr));

        /* Get number of lines*/

        int num = rand()%num_lines;
        fprintf(stdout, "Random line: %d\n", num);
        echolen = sizeof(int);
        if (sendto(udp_sock, &num, sizeof(int), 0, (struct sockaddr *) &udp_server, sizeof(udp_server)) != echolen) {
            err_sys("error writing word on socket");
        }
        clientlen = sizeof(udp_client);
        if (recvfrom(udp_sock, &num, sizeof(int), 0, (struct sockaddr *) &udp_client, &clientlen) != sizeof(int)) {
            err_sys("Error reading");
        }
        fprintf(stdout, "Answer: %d\n", num);
        /* Call function to handle socket */
        handle_client(clientsock, num);
  }
}

int check_port(char *port_src){
    
    int port = atoi(port_src);
    return port > 0 && port < 65536;
}


void handle_client(int sock, int answer) {
  char buffer[BUFFSIZE];
  int received = -1;
  int guess, num_guesses = 0;
  
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