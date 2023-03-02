#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFSIZE 255
#define DEFAULTPORT 9999
#define MAX_LINE_LENGTH 256
#define MAX_LINES 1024

void err_sys(char *mess) { perror(mess); exit(1); }

int main(int argc, char *argv[]) {
    struct sockaddr_in echoserver, echoclient;
    unsigned int echolen, clientlen, serverlen;
    char buffer[BUFFSIZE];
    int message = 0;
    int sock, received = 0;
    
    int first_request = 1;
    int port = DEFAULTPORT;
    int num_lines = 0;

    /* Check input arguments */
    if (argc > 3 || argc <= 1) {
        fprintf(stderr, "Usage: %s <file_name> <port>\n", argv[0]);
        exit(1);
    }
    else if(argc > 2){
        if(!check_port(argv[2])){
            printf("Invalid port number: %s\n", argv[2]);
            exit(1);
        }
        port = atoi(argv[2]);
    }

    /*Save file info*/
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        err_sys("Error opening file: " + *argv[1]);
    }
    int letters[MAX_LINES];
    char line[MAX_LINE_LENGTH];

    while(fgets(line, MAX_LINE_LENGTH, fp) != NULL && num_lines < MAX_LINES){
        letters[num_lines] = (strlen(line) - 1)%100;
        num_lines++;
    }
    fclose(fp);

    /* Create UDP socket */
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        err_sys("Error socket");
    }

    /* Prepare sockaddr_in structure for server address */
    memset(&echoserver, 0, sizeof(echoserver));       /* Erase the memory area */
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY); /* Receive requests from any IP address valid on server */
    echoserver.sin_port = htons(port);       /* Server port */

    /* Get size of echoserver structure */
    serverlen = sizeof(echoserver);

    /* Bind that socket with the OS, to be able to receive messages on that socket */
    if (bind(sock, (struct sockaddr *) &echoserver, serverlen) < 0) {
        err_sys("Error bind");
    }

    /* As a server we are in an infinite loop, waiting forever */
    while (1) {
        /* Set the maximum size for address */
        clientlen = sizeof(echoclient);
        echolen = sizeof(int);
        /* Receive a message from a particular client */
        if (first_request) {
            received = recvfrom(sock, buffer, BUFFSIZE, 0, (struct sockaddr *) &echoclient, &clientlen);
            if (received < 0) {
                err_sys("Error receiveing word from client");
            }
            buffer[received] = '\0';

            /* Print client address */
            fprintf(stderr, "Client: %s, Message: %s\n", inet_ntoa(echoclient.sin_addr), buffer);

            sendto(sock, &num_lines, echolen, 0, (struct sockaddr *) &echoclient, clientlen);
            first_request = 0;
        } else {
            received = recvfrom(sock, buffer, sizeof(int), 0, (struct sockaddr *) &echoclient, &clientlen);
            if (received != sizeof(int)) {
                err_sys("Error client didnt send an integer");
            }
            memcpy(&message, buffer, sizeof(int));
            /* Print client number */
            fprintf(stderr, "Client: %s, Message: %d\n", inet_ntoa(echoclient.sin_addr), message);
            sendto(sock, &letters[message], echolen, 0, (struct sockaddr *) &echoclient, clientlen);
            first_request = 1;
        }
    }
}
int check_port(char *port_src){
    
    int port = atoi(port_src);
    return port > 0 && port < 65536;
}