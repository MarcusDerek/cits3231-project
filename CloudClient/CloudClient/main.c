//
//  main.c
//  CloudClient
//
//  Created by Marcus Derek Phua on 14/10/2012.
//  Copyright (c) 2012 Internix Cloud Providers All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "clientCommands.h"

#define CLOUD_SERVER_PORT "9999"
#define CLOUD_SERVER_ADDRESS "192.168.211.128"
#define MAXDATASIZE 100;

/**
 * For getting socket address, IPv4 or IPv6
 * @param sa
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int receiveDataFrom(int sockfd, char* received_packet, int received_packet_size) {
    int success = 0;
    int bytes_received = 0;
    while (bytes_received < received_packet_size) {
        bytes_received = recv(sockfd, received_packet, received_packet_size, 0);
        if(bytes_received != 0) { //TEMP BUG FIX
            success = 1;
            break;
        } else {
            printf("Connection terminated abruptly.\n");
            exit(EXIT_FAILURE);
        }
    }
    printf("Client: Packet received successful.\n");
    return success;
    
}
int sendDataTo(int sockfd, char* sent_packet, int sent_packet_size) {
    int success = 0;
    int bytes_sent = 0;
    while (bytes_sent < sent_packet_size) {
        bytes_sent = send(sockfd, sent_packet, sent_packet_size, 0);
    }
    if(bytes_sent >= sent_packet_size) {
        success = 1;
    }
    printf("Client: Packet sent successful.\n");
    return success;
}
/**
 * 
 * @param sockfd
 */
void communicateWithCloudServer(int sockfd) {
    char *packet_data = malloc(1000 * sizeof(char));
    
    printf("Input Command: ");
    char *userCommand = malloc(100 * sizeof(char));
    scanf("%s", userCommand);
    
    /**
     * Used for verifying the user Command
     */
    int option = 404; //Default error
    option = verifyUserCommand(userCommand);
    if(option == 0) { //-exit
        printf("Thanks for choosing Internix Cloud!\nHope to see you soon!\n");
        exit(EXIT_SUCCESS);
    } else if(option == 1) { //-registerNewAccount
        strcpy(packet_data, registerNewAccount());
        printf("Packet Data = %s\n", packet_data);
        
        int packet_size = strlen(packet_data);
        if(sendDataTo(sockfd, packet_data, packet_size) == 1) {//Success SEND
            char *received_packet = malloc (1000 * sizeof(char));
            if(receiveDataFrom(sockfd, received_packet, 30000) == 1) {//Success RECEIVE 
                printf("Received data: %s\n", received_packet);
            }
        }else{
           printf("Error: Sending packet. -registerNewAccount\n"); 
        } 
    }
}
/**
 * 
 */
void connectToCloudServer() {
    int sockfd, numbytes;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
	
    memset(&hints, 0, sizeof hints); //Important Step 1 - Clear all struct to make sure empty
    hints.ai_family = AF_INET; //Important Step 2 - NON-SPECIFIC Ipv4 or Ipv6 -INET = IPV4
    hints.ai_socktype = SOCK_STREAM; //Important Step 3 - TCP Stream Packets declaration
    
	printf("Stage 1 completed - Variables Initialised.\n");
	
	//Getting ready to connect
	//Getinfo Command  0 return == FAIL
	//If All goes well, servinfo will contain a linked list of struct addrinfo!. see above
    if ((rv = getaddrinfo(CLOUD_SERVER_ADDRESS, CLOUD_SERVER_PORT, &hints, &servinfo)) != 0) { 
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }
    
    printf("Stage 2 completed - getAddrInfo Success.\n");
    
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
		//ALWAYS FEED SOCKET with ADDR Info from above!!!!
        if ((sockfd = socket(p->ai_family, p->ai_socktype, //Calling for Socket Descriptor
                             p->ai_protocol)) == -1) {
            perror("client: socket error"); //else
            continue;
        }
        printf("Stage 3 completed - SocketFD success, ID = %d.\n", sockfd);
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        { //Calling for Connect
            close(sockfd);
            perror("client: connection error"); //else NO CRASH
            exit(EXIT_FAILURE); //Fail if cannot connect
            //continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "Client: Failed to connect\n");
        exit(EXIT_FAILURE);
    }
    
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("Cloud Client: Connecting to %s\n\n\n", s);
    
    freeaddrinfo(servinfo); // all done with this structure
    
    /**
     * This section is used to response to server connections
     * All actions MUST BE executed within the WHILE LOOP
     */
    printf("You are now connected to: Orion-Cloud-Server\n\n\n");
    while(1) {
        communicateWithCloudServer(sockfd);
    }
    
    printf("Connection ended successfully.\n");
}



/*
 * 
 */
int main(int argc, char** argv) {
    
    char *introMsg = get_IntroMsg();
    char *userInputCommand = malloc (1000 * (sizeof(char)));
	char *userName = malloc(1000 * (sizeof(char)));
	char *password = malloc(1000 * (sizeof(char)));
	char *output = malloc(1000 * sizeof(char));
    int verifyCode = 0;
    
    printf(introMsg);
    while (1) {
		connectToCloudServer();
    }   
    return (EXIT_SUCCESS);
}

