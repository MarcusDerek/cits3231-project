//
//  main.c
//  CloudServer
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
#include <sys/stat.h>



#define CLOUD_SERVER_PORT "3230"  // the port users will be connecting to
#define CLOUD_SERVER_ADDRESS "130.95.1.70"
#define TESTPORT "9999" //Port for simulating cloud server
#define BACKLOG 10     // how many pending connections queue will hold
#define FILESERVER_AREA "/Users/MarcusDerek/Documents/UWA/Storage"
#define MAX_FILE_SIZE 50000

typedef struct {
    char* file_buffer;
    char* file_path;
    long size_of_file;			
} FILE_DATA;
/**
 * Essential function for setting up the Cloud Server
 * For getting socket address, IPv4 or IPv6
 * @param s
 */
void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}
/**
 * Essential function for setting up the Cloud Server
 * @param s
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
char* receive_all_data(int sockfd, int packet_size) {
    int bytes_received;
    int total_size = 0;
    char *packet = malloc(packet_size * sizeof(char));
    while(bytes_received != packet_size) {
        bytes_received = recv(sockfd, packet + total_size, packet_size, 0); //Receive First Line @ Loop 0
        total_size = total_size + bytes_received; //Increment pointer to add additional data
        printf("%d bytes received.\n", total_size);
        if(bytes_received == 0) {//Problem or Done
                printf("File transfer complete.\n");
                break;
        }
        if(bytes_received == -1) {
                printf("Error receiving File.\n");
                break;
        }
    }
    printf("Total Bytes Received from Server: %d\n", total_size);
    return packet;
}

void packetToFile(char *packet, int size, char *fileName) { 
    //Extract File
    FILE *pFile;
    char *storageFilePath = malloc(1000 * sizeof(char));
    strcat(storageFilePath,"Storage/");
    strcat(storageFilePath,fileName);
    pFile = fopen(storageFilePath, "wb");
    fwrite(packet, 1, size, pFile);
    fclose(pFile);
    printf("File created.\n");
}
/**
 * BRYAN TO DO
 * @param userName
 */
void createUserHomeDirectory(char* userName) {
   char* fullPathName = malloc(1000 * sizeof(char));
   sprintf(fullPathName,"%s/%s", FILESERVER_AREA, userName);
   mkdir(fullPathName,0777);
   printf("Created Directory at: %s\n", fullPathName);
}
/**
 * 
 * @param fileNameSize
 * @param filesize
 * @param fileName
 */
void extractFileNameSize(char *fileNameSize, int *filesize, char *fileName) {
    char *tempFileSize = malloc(100 * sizeof(char));
    sscanf(fileNameSize, "%*d %s %s",tempFileSize, fileName);
    *filesize = atoi(tempFileSize);
}
int receiveDataFrom(int sockfd, char* received_packet, int received_packet_size) {
    int bytes_received;
    int total_size = 0;
    int success = 0;
    while(bytes_received != received_packet_size) {
        bytes_received = recv(sockfd, received_packet + total_size, received_packet_size, 0); //Receive First Line @ Loop 0
        total_size = total_size + bytes_received; //Increment pointer to add additional data
        printf("%d bytes received.\n", total_size);
        if(bytes_received == 0) {
                printf("File transfer complete.\n");
                break;
        }
        if(bytes_received == -1) {
                printf("Error receiving file.\n");
                break;
        }
        if(total_size == received_packet_size) {
            success = 1;
            break;
        }
    }
    printf("Total Bytes Received from Server: %d\n", total_size);
    return success;
}
int receiveFileNameSizeFrom(int sockfd, char *received_packet) {
    recv(sockfd, received_packet, 30, 0);
}
/**
 * 
 * @param sockfd
 * @param received_packet
 * @return 
 */
int receiveCommandFrom(int sockfd, char* received_packet) {
    int success = 1;
    int bytes_received = recv(sockfd, received_packet, 30, 0);
    if(bytes_received == 0) {
        printf("Connection terminated abruptly.\n");
        exit(EXIT_FAILURE);
    }
    return success;
}
/**
 * 
 * @param sockfd
 * @param sent_packet
 * @param sent_packet_size
 * @return 
 */
int sendDataTo(int sockfd, char* sent_packet, int sent_packet_size) {
    int success = 0;
    int bytes_sent = 0;
    while (bytes_sent < sent_packet_size) {
        bytes_sent = send(sockfd, sent_packet, sent_packet_size, 0);
    }
    if(bytes_sent >= sent_packet_size) {
        success = 1;
    }
    printf("Server: Packet sent successful.\n");
    return success;
}

int processRegisterNewAccount(char* received_packet, int new_fd) {
    char *userName = malloc(100 * sizeof(char));
    char *password = malloc(100 * sizeof(char));
    sscanf(received_packet, "%*d %s %s", userName, password);
    printf("Registering user - %s\n", userName);
    createUserHomeDirectory(userName); //BRYAN - return pass or fail
    
    int pass = 1;
    int fail = 0;
    return pass; //NEED TO CHANGE
}
int loginToAccount(char *received_packet, int new_fd) {
    return 1;
}
int addFile(char* received_packet, int new_fd) {
    //int status = convertPacketToFile(received_packet);
}
int addFileToCloud(int new_fd) {
    int success = 0;
    printf("Prepared to receive FILE SIZE.\n");
    char *fileNameSize = malloc(1000 * sizeof(char));
    int file_size;
    int bytes_received;
    receiveFileNameSizeFrom(new_fd, fileNameSize);
    char *fileName = malloc(1000 * sizeof(char));
    extractFileNameSize(fileNameSize, &file_size, fileName);
    printf("Filename: %s | Size: %d\n",fileName, file_size);
    char *file_packet = malloc(file_size * sizeof(char));
    if(receiveDataFrom(new_fd, file_packet, file_size) == 1) { //success
        packetToFile(file_packet, file_size, fileName);
        success = 1;
    } else {
        printf("Error occurred: addFileToCloud()\n");
        exit(EXIT_FAILURE);
    }
    return success;
}

int main(int argc, char** argv) {
    
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP ~ MAY HAVE TO CHANGE FOR UWA SERVER
    
    if ((rv = getaddrinfo(NULL, TESTPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
	printf("Binding IP..............................\n");
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    printf("Freeing ServeInfo..............................\n");
    
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    printf("Freeing Dead Processes..............................\n\n");
    
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
	
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    printf("Cloud Server: Online\n\n");
    printf("Server status: Awaiting connections from Port: %s\n", TESTPORT);
    //***************************************************************************
    /*
     Server awaits for connection after this line. (For Testing only)
     Set variables to store. Below variables are allocated only ONCE.
     */
	
    //***************************************************************************
    /*
     WHILE LOOP - for continuosly accepting incoming connections
     For every new connection, a new FILE DESCRIPTOR (new_fd) is issued.
     */
    
    while(1) {  // main accept() loop
        
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); //Accepts an incoming transmission
        
        if (new_fd == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);
        printf("Awaiting User Commands...\n");
        
        /**
         * This section is used to response to server connections
         * Do everything in the WHILE loop to maintain session
         */
        while(1) {
            char *received_packet = malloc(1000 * sizeof(char));
            int file_size;
            char *sent_packet = malloc(1000 * sizeof(char));

            if(receiveCommandFrom(new_fd, received_packet) == 1) {
                printf("Received MSG: %s\n", received_packet);
            } else {
                printf("Error: Receive invalid command!\n");
                exit(EXIT_FAILURE);
            }
            int userInputCommand;
            /*
             *  Exceptions
             */
             /*
             * Determine which command is used
             */
            
            int status;
            sscanf(received_packet, "%d", &userInputCommand);
            /**
            * This section is used to validate the commands
            */
            switch (userInputCommand) {
                case 1:
                    status = processRegisterNewAccount(received_packet, new_fd);
                    if(status == 1) { //1 == Pass, 0 == fail
                        sendDataTo(new_fd, "1", 10);
                        printf("-registerNewAccount: Success\n");
                    } else {
                        sendDataTo(new_fd, "0", 10);
                        printf("-registerNewAccount: Fail\n");
                    }
                    break;
                case 2:
                    status = loginToAccount(received_packet, new_fd);
                    if(status == 1) { //1 == Pass, 0 == fail
                        sendDataTo(new_fd, "1", 10);
                        printf("-login: Success\n");
                    } else {
                        sendDataTo(new_fd, "0", 10);
                        printf("-login: Fail\n");
                    }
                    break;
                case 3:
                    status = addFileToCloud(new_fd);
                    if(status == 1) { //1 == Pass, 0 == fail
                        sendDataTo(new_fd, "1", 10);
                        printf("-addFile: Success\n");
                    } else {
                        sendDataTo(new_fd, "0", 10);
                        printf("-addFile: Fail.\n");
                    }
                    break;
                    
                    //status = addFile(received_packet, new_fd);
            }
                    
        
        } //End of session while loop
		
        
    }//End of server response
}


