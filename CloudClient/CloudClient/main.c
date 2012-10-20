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
#include "tpl.h"

#define CLOUD_SERVER_PORT "9999"
#define CLOUD_SERVER_ADDRESS "192.168.211.130"
#define MAXDATASIZE 100;
#define ERROR "ERROR"
#define MAX_FILE_SIZE 50000000

typedef struct {
    char* file_buffer;
    char* file_path;
    long size_of_file;			// e.g.		-65
} FILE_DATA;


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
 * Compress ANY file into CHAR and send it across the network
 * @param file_size - Pass reference point to store size. Used for send()
 * @param file_path - FULL path of file
 * @return char array of compressed data
 */
char* compress_File_to_stream(long *file_size, char *file_path)
{
	FILE *pFile;
	pFile = fopen(file_path, "rb");

	if(pFile == NULL)
	{
	 	fprintf(stderr,"Error opening file.\n");
	    exit(EXIT_FAILURE);
	}

	//OBTAIN SIZE OF THE FILE
	long lSize;
	fseek(pFile, 0, SEEK_END); //Point the stream indicator all the way to the end of file
	lSize = ftell(pFile); //Returns the size of the file based on the end indicator
	rewind(pFile); //Point the stream indicator BACK to the start
	file_size = &lSize;

	//ALLOCATE MEMORY FOR FILE
	char *buffer;
	buffer = (char*) malloc (sizeof(char) * lSize); //Malloc memory
	if (buffer == NULL)
	{
		fprintf(stderr,"Error allocating memory to file.\n");
		exit(EXIT_FAILURE);
	}

	//LOAD FILE INTO BUFFER
	size_t result;
	result = fread(buffer, 1, lSize, pFile);
	printf("Size of file is %lu\n", result);
	if(result != lSize)
	{
	    fprintf(stderr,"Error loading file into buffer.\n");
	    exit(EXIT_FAILURE);
	}
	fclose(pFile);
	return buffer;
}
;
char* processExitOption() {
    char *exitMsg = "Thanks for choosing Internix Cloud!\nHope to see you soon!\n";
    return exitMsg;
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
    int packet_size;
    long file_size;
    int option = 404; //Default error
    option = verifyUserCommand(userCommand);
    
    switch (option) {
        case 0: //-exit
           printf("%s", processExitOption());
           exit(EXIT_SUCCESS);
           break;
        case 1: //-registerNewAccount
            strcpy(packet_data, registerNewAccount());
            if(strcmp(packet_data, ERROR) == 0) {
                printf("Error in CMD 1.\n");
                return;
            }
            printf("Packet Data = %s\n", packet_data);
            packet_size = strlen(packet_data);
            if(sendDataTo(sockfd, packet_data, packet_size) == 1) {//Success SEND
                char *received_packet = malloc (1000 * sizeof(char));
                if(receiveDataFrom(sockfd, received_packet, 30000) == 1) {//Success RECEIVE REPLY
                    printf("Received data: %s\n", received_packet);
                }
            }else{
                printf("Error: Sending packet. -registerNewAccount\n"); 
            }
            break;
        case 2://-login
            strcpy(packet_data, loginToAccount());
            if(strcmp(packet_data, ERROR) == 0) {
                printf("Error in CMD 1.\n");
                return;
            }
            printf("Packet Data = %s\n", packet_data);
            packet_size = strlen(packet_data);
            if(sendDataTo(sockfd, packet_data, packet_size) == 1) {//Success SEND
                char *received_packet = malloc (1000 * sizeof(char));
                if(receiveDataFrom(sockfd, received_packet, 30000) == 1) {//Success RECEIVE REPLY
                    printf("Received data: %s\n", received_packet);
                }
            }else{
                printf("Error: Sending packet. -login\n"); 
            }
            break;
        case 3: //-addFile
            file_size;
            FILE_DATA *newFile = malloc(sizeof(FILE_DATA));
            newFile->file_buffer = malloc(MAX_FILE_SIZE * sizeof(char));
            newFile->file_path = malloc(5000 * sizeof(char));
            strcpy(newFile->file_path, addFile());
            strcpy(newFile->file_buffer, compress_File_to_stream(&file_size, newFile->file_path));
            newFile->size_of_file = file_size;
            
            tpl_node tn;
            tn = tpl_map("S(sii)", &newFile);
            tpl_pack(tn, 0);
            char filePacket[MAX_FILE_SIZE];
            tpl_dump(tn, TPL_MEM, filePacket, MAX_FILE_SIZE);
            
            if(sendDataTo(sockfd, filePacket, file_size) == 1) {//Success SEND
                tpl_free(tn);
                char *received_packet = malloc (1000 * sizeof(char));
                if(receiveDataFrom(sockfd, received_packet, 30000) == 1) {//Success RECEIVE REPLY
                    printf("Received data: %s\n", received_packet);
                }
            }else{
                printf("Error: Sending packet. -addFile\n"); 
            }
            break;
        case 4: //-deleteFile
            break;
        case 5: //-fetchFile
            break;
        case 6: //-verifyFile
            break;
            
                
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
    
    printf("%s", introMsg);
    while (1) {
		connectToCloudServer();
    }   
    return (EXIT_SUCCESS);
}

