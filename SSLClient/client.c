/* 
 * File:   main.c
 * 
 * Team: 
 * Marcus Derek - 11016403
 * Taiga Yano - 20698782
 * Bryan Kho - 20714477
 * 
 * CLIENT
 *
 */

//#include "openssl/include/openssl/bio.h"
//#include "openssl/include/openssl/ssl.h"
//#include "openssl/include/openssl/err.h"
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
//SSL-Client.c  
#include <stdio.h>  
#include <errno.h>  
#include <unistd.h>   
#include <string.h>  
#include <sys/socket.h>  
#include <resolv.h>  
#include <netdb.h>
#include "commands.h"
  
#define CLOUD_SERVER_PORT "9999"
#define CLOUD_SERVER_ADDRESS "192.168.211.130"
#define DEFAULT_SIZE 100
#define ERROR 404
#define MAX_FILE_SIZE 50000
#define FAIL    -1  


/*Global Variable*/
char LOGGED_IN_AS_USERNAME[100]; 


/* -------------- BASE CODE - DO NOT TOUCH *----------------------------------------------------------*/
int OpenConnection(const char *hostname, int port)  {
    int sd;  
    struct hostent *host;  
    struct sockaddr_in addr;  
  
    if ( (host = gethostbyname(hostname)) == NULL )  {  
        perror(hostname);  
        abort();  
    }  
    sd = socket(PF_INET, SOCK_STREAM, 0);  
    bzero(&addr, sizeof(addr));  
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(port);  
    addr.sin_addr.s_addr = *(long*)(host->h_addr);  
    if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )  {  
        close(sd);
        perror(hostname);  
        abort();  
    }  
    return sd;  
}  
  
SSL_CTX* InitCTX(void)  
{   SSL_METHOD *method;  
    SSL_CTX *ctx;  
  
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */  
    SSL_load_error_strings();   /* Bring in and register error messages */  
    method = SSLv2_client_method();  /* Create new client-method instance */  
    ctx = SSL_CTX_new(method);   /* Create new context */  
    if ( ctx == NULL )  {  
        ERR_print_errors_fp(stderr);  
        abort();  
    }  
    return ctx;  
}  
/**
 * Modified to include cert verification
 * @param ssl
 */  
void ShowBankCerts(SSL* ssl)  {
    X509 *cert;  
    char *line;
    char commonName[512];
    char hostname[] = "Cloud Bank";
  
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */  
    X509_NAME * name = X509_get_subject_name(cert);
    X509_NAME_get_text_by_NID(name, NID_commonName, commonName, 512);
    if(strcmp(commonName, hostname) != 0) {
        printf("Invalid certificate detected!.\n");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Certificate valid. Host is verified.\n");
    }  
        
}
/**
 * Modified to include cert verification
 * @param ssl
 */  
void ShowServerCerts(SSL* ssl)  {
    X509 *cert;  
    char *line;
    char commonName[512];
    char hostname[] = "CloudServer";
  
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */  
    X509_NAME * name = X509_get_subject_name(cert);
    X509_NAME_get_text_by_NID(name, NID_commonName, commonName, 512);
    if(strcmp(commonName, hostname) != 0) {
        printf("Invalid certificate detected!.\n");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Certificate valid. Host is verified.\n");
    }
        
} 
/* -------------- BASE CODE - DO NOT TOUCH *----------------------------------------------------------*/

/**
 * Compress file into a buffer to send across the network
 * @param file_size size of file
 * @param file_path full file path
 * @return the buffer containing the compressed file
 */
char* compress_File_to_stream(size_t *file_size, char *file_path)
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
        *file_size = result;
	if(result != lSize)
	{
	    fprintf(stderr,"Error loading file into buffer.\n");
	    exit(EXIT_FAILURE);
	}
	fclose(pFile);
        printf("File Process Complete.\n");
	return buffer;
}
/**
 * Prints the exit message
 * @return the exit message
 */
char* processExitOption() {
    char *exitMsg = "Thanks for choosing Internix Cloud!\nHope to see you soon!\n";
    return exitMsg;
}
/**
 * Sends primitive data to X location
 * Use send_all_data() for sending big files
 * @param ssl SSL socket
 * @param sent_packet - packet data
 * @param sent_packet_size - size of packet
 * @return 1 = success 0 otherwise
 */
int sendDataTo(SSL *ssl, char* sent_packet, int sent_packet_size) {
    int success = 0;
    int bytes_sent = 0;
    while (bytes_sent < sent_packet_size) {
        bytes_sent = SSL_write(ssl, sent_packet, sent_packet_size);
    }
    if(bytes_sent >= sent_packet_size) {
        success = 1;
    }
    //printf("Client: Packet sent successful.\n");
    return success;
}
/**
 * Use for sending files only
 * @param ssl - SSL packet
 * @param packet - packet data
 * @param packet_size - size of packet
 * @return 0 succes, 1 otherwise (NOTE ITS OPPOSITE!!)
 */
int send_all_data(SSL *ssl, char *packet, int packet_size)
{
    int bytes_sent = 0;        // how many bytes we've sent
    int bytes_remaining = packet_size; // how many we have left to send
    int n;

    while(bytes_sent < packet_size) {
        n = SSL_write(ssl, packet + bytes_sent, bytes_remaining);
        //printf("%d bytes sent to client.\n", n);
        if (n == -1) { 
            printf("BREAK!!!!\n");
            break; 
        }
        bytes_sent = bytes_sent + n;
        bytes_remaining = bytes_remaining - n;
    }
	//printf("Total bytes sent = %d\n", bytes_sent);

    return n==-1?-1:0; // return -1 on failure, 0 on success
}
/**
 * To receive primitive data. Use receive_all_data for large files
 * @param ssl - ssl socket
 * @param received_packet - packet data
 * @param received_packet_size - size of packet
 * @return 1 success, 0 otherwise
 */
int receiveDataFrom(SSL *ssl, char* received_packet, int received_packet_size) {
    int success = 0;
    int bytes_received = 0;
    while (bytes_received < received_packet_size) {
        bytes_received = SSL_read(ssl, received_packet, received_packet_size);
        if(bytes_received != 0) { //TEMP BUG FIX
            success = 1;
            break;
        } else {
            printf("Connection terminated abruptly.\n");
            exit(EXIT_FAILURE);
        }
    }
    //printf("Client: Packet received successful.\n");
    return success;
    
}
/**
 * Receive Data from Cloud. Use for files
 * @param ssl - ssl socket
 * @param received_packet - packet data
 * @param received_packet_size - size of packet
 * @return 1 success, 0 otherwise
 */
int receiveFileFrom(SSL *ssl, char* received_packet, int received_packet_size) {
    int bytes_received;
    int total_size = 0;
    int success = 0;
    while(bytes_received != received_packet_size) {
        bytes_received = SSL_read(ssl, received_packet + total_size, received_packet_size); //Receive First Line @ Loop 0
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
/**
 * 
 * @param filepath
 * @return 
 */
char *extractFileName(char *filePath) {
    char *copyFilePath = malloc(1000 * sizeof(char));
    strcpy(copyFilePath, filePath);
    char *pch;
    char *fileName;
    pch=strchr(copyFilePath,'/');
    while (pch!=NULL) {
      pch=strchr(pch+1,'/');
      if(pch !=NULL) {
          fileName = pch;
          fileName++;
      }
    }
    printf("Actual filename = %s\n", fileName);
    return fileName;
}
/**
 * Sends the File size and name to the Server
 * @param ssl
 * 
 */
void sendFileSizeNameDataTo(SSL *ssl, char *sent_packet) {
    
    int bytes_sent = SSL_write(ssl, sent_packet,30);
}
int sendFileToServer(SSL *ssl) {
    superStringSend(ssl, 5, "3");
    //sendDataTo(ssl, "3", 5); //Trigger server to receive file
    char *filePath = malloc(1000 * sizeof(char));
    size_t file_size;
    strcpy(filePath, addFile());
    extractFileName(filePath);
    char *fileBuffer = compress_File_to_stream(&file_size, filePath);
    char *convertFileSize = malloc(100 *sizeof(char));
    sprintf(convertFileSize,"%lu", file_size);
    char *fileNameSize = concatSentence(0, convertFileSize, extractFileName(filePath)); //0 = null
    sendFileSizeNameDataTo(ssl, fileNameSize); //Sends the FILE size and Name to Server. (To use for checking BANK ACCOUNT)
    if(send_all_data(ssl, fileBuffer, file_size) == 0) { //Success
        char *received_packet = malloc (1000 * sizeof(char));
        if(receiveDataFrom(ssl, received_packet, 30000) == 1) {//Success HANDLE REPLY
            if(strcmp(received_packet,"1") == 0) {
                return 1;
            }
            else {
                return 0;
            }
        }
    }
                    
}
int receiveFileNameSizeFrom(SSL *ssl, char *received_packet) {
    SSL_read(ssl, received_packet, 30);
}
/** 
 * @param fileNameSize
 * @param filesize
 * @param fileName
 */
void extractFileNameSize(char *fileNameSize, int *filesize, char *fileName) {
    char *tempFileSize = malloc(100 * sizeof(char));
    sscanf(fileNameSize, "%*d %s %s",tempFileSize, fileName);
    *filesize = atoi(tempFileSize);
}
/**
 * Packs the file into the proper working format
 * @param packet - the data packet
 * @param size - size of file
 * @param fileName - name of file
 */
void packetToFile(char *packet, int size, char *fileName) { 
    //Extract File
    FILE *pFile;
    char *storageFilePath = malloc(1000 * sizeof(char));
    //strcat(storageFilePath,"Storage/"); //Default NO STORAGE
    strcat(storageFilePath,fileName);
    pFile = fopen(storageFilePath, "wb");
    fwrite(packet, 1, size, pFile);
    fclose(pFile);
    printf("File created.\n");
}
/**
 * Used for receiving large file
 * @param ssl ssl socket
 * @param packet file packet consisting of filename
 * @return 1 success, 0 otherwise
 */
int receiveFileFromServer(SSL *ssl, char* packet) {
    sendDataTo(ssl, packet, strlen(packet)); //Trigger server to receive file
    int success = 0;
    printf("Prepared to receive FILE SIZE.\n");
    char *fileNameSize = malloc(1000 * sizeof(char));
    int file_size;
    int bytes_received;
    receiveFileNameSizeFrom(ssl, fileNameSize); //Receive filename and size from SERVER
    char *fileName = malloc(1000 * sizeof(char));
    extractFileNameSize(fileNameSize, &file_size, fileName);
    printf("Filename: %s | Size: %d\n",fileName, file_size);
    char *file_packet = malloc(file_size * sizeof(char));
    if(receiveFileFrom(ssl, file_packet, file_size) == 1) { //RECEIVING ACTUAL FILE DATA
        packetToFile(file_packet, file_size, fileName);
        success = 1;
    } else {
        printf("Error occurred: addFileToCloud()\n");
        exit(EXIT_FAILURE);
    }
    return success;
}
int deleteFileFromServer(SSL *ssl) {
    char *packet_data = malloc(1000 * sizeof(char));
    strcpy(packet_data, deleteFile()); //Return filename to be deleted <4 filename "">
    int packet_size = strlen(packet_data);
    if(superStringSend(ssl, packet_data, packet_size) == 1) { //Success SEND
        char *received_packet = malloc (1000 * sizeof(char));
        if(receiveDataFrom(ssl, received_packet, 30000) == 1) {//Success HANDLE REPLY
            if(strcmp(received_packet,"1") == 0) {
                return 1; //success
            }
            else {
                return 0; //fail
            }
        }
    }else{
        return 0; //fail
    }
}
int verifyFileOnServer(SSL *ssl) {
    char *packet_data = malloc(1000 * sizeof(char));
    strcpy(packet_data, verifyFile()); //Return filename to be deleted <4 filename "">
    int packet_size = strlen(packet_data);
    if(sendDataTo(ssl, packet_data, packet_size) == 1) { //Success SEND
        char *received_packet = malloc (1000 * sizeof(char));
        if(receiveDataFrom(ssl, received_packet, 30000) == 1) {//Success HANDLE REPLY
            if(strcmp(received_packet,"1") == 0) {
                return 1; //success
            }
            else {
                return 0; //fail
            }
        }
    }else{
        return 0;// fail 
    }
}
int listAllFilesOnServer(SSL *ssl) {
    char *packet_data = malloc(1000 * sizeof(char));
    strcpy(packet_data, listAllFiles()); //Return command to retrieve list of files
    int packet_size = strlen(packet_data);
    if(superStringSend(ssl, packet_size, packet_data) == 1) { //Success SEND
        char *received_packet = malloc (100000 * sizeof(char)); //PREPARE TO RECEIVE LIST OF FILES. MIGHT BE HUGE
        if(superStringReceive(ssl, received_packet) == 1) {//Success HANDLE REPLY
            printf("%s", received_packet);
            return 1;
        }
    }else{
        return 0;// fail 
    }
}
/**
 * Super improved string sending function
 * @param sockfd - socket
 * @param sizeOfDataInfo - Size of the string in a string
 * @param output - The actual string to be sent
 * @return 
 */
int superStringSend(SSL *ssl, int sizeOfDataInfo, char* output) {
    char *sizeofdata = malloc(1000 * sizeof(char));
    int size = strlen(output);
    sprintf(sizeofdata,"%d", size);
    send_all_data(ssl, sizeofdata,1000);
    send_all_data(ssl, output, strlen(output));
    return 1; //Temp 
}
char* receive_all_data(SSL *ssl, int packet_size) {
    int bytes_received;
    int total_size = 0;
    char *packet = malloc(packet_size * sizeof(char));
    while(bytes_received != packet_size) {
        //bytes_received = recv(sockfd, packet + total_size, packet_size, 0); //Receive First Line @ Loop 0
        bytes_received = SSL_read(ssl, packet + total_size, packet_size);
        total_size = total_size + bytes_received; //Increment pointer to add additional data
        //printf("%d bytes received.\n", total_size);
        if(bytes_received == 0) {//Problem or Done
                printf("File transfer complete.\n");
                break;
        }
        if(bytes_received == -1) {
                printf("Error receiving File.\n");
                break;
        }
    }
    //printf("Total Bytes Received from Server: %d\n", total_size);
    return packet;
}
int superStringReceive(SSL *ssl, char* received_packet) {
    char *size = malloc(1000 * sizeof(char));
    size = receive_all_data(ssl, 1000);
    int sizeOfPacket = atoi(size);
    char *buffer = malloc(1000 * sizeof(char));
    buffer = receive_all_data(ssl, sizeOfPacket);
    char *mrNullPtr = "\0";
    strncpy(received_packet, buffer, sizeOfPacket);
    strcat(received_packet, mrNullPtr);
    return 1;
}
/**
 * Access to this functions is allowed for anyone. No logged in required
 * @param ssl
 * @return 1 successful login, 0, fail 
 */
int processCommonInputs(SSL *sslServer, SSL *sslBank) {
    int logged_in = 0; //0 = false
    char *packet_data = malloc(1000 * sizeof(char));
    printf("Input Command: ");
    char *userCommand = malloc(100 * sizeof(char));
    scanf("%s", userCommand);
    /**
     * Used for verifying the user Command
     */
    int packet_size;
    int option = 404; //Default error
    option = verifyUserCommand(userCommand);
    if(option == ERROR) {
        printf("Invalid Command. - Please retype your command or try -help.\n");
        logged_in = 0;
        return logged_in;
    }
    
    switch (option) {
        case 0: {//-exit
           printf("%s", processExitOption());
           exit(EXIT_SUCCESS);
           break;
        }
        case 1: {//-registerNewAccount
            strcpy(packet_data, registerNewAccount());
            printf("Packet Data = %s\n", packet_data);
            packet_size = strlen(packet_data);
            //if(sendDataTo(sslServer, packet_data, packet_size) == 1) {//Success SEND
            if(superStringSend(sslServer, packet_size, packet_data) == 1) {
                char *received_packet = malloc (1000 * sizeof(char));
                if(receiveDataFrom(sslServer, received_packet, 30000) == 1) {//Success RECEIVE REPLY
                    if(strcmp(received_packet,"1") == 0) {
                        system("clear");
                        printf("Account registration successful!\n");
                    }
                    else {
                        system("clear");
                        printf("Sorry, the username has already been taken!.\n");
                    }
                }
            }else{
                printf("Error: Sending packet. -registerNewAccount\n"); 
            }
            break;
        }
        case 2: {//-login
            strcpy(packet_data, loginToAccount());
            printf("Packet Data = %s\n", packet_data);
            packet_size = strlen(packet_data);
            //if(sendDataTo(sslServer, packet_data, packet_size) == 1) {//Success SEND
            if(superStringSend(sslServer, packet_size, packet_data) == 1) {
                char *received_packet = malloc (1000 * sizeof(char));
                if(receiveDataFrom(sslServer, received_packet, 30000) == 1) {//Success HANDLE REPLY
                    if(strcmp(received_packet,"1") == 0) {
                        printf("Login successful!\n");
                        sscanf(packet_data, "%*d %s %*s", LOGGED_IN_AS_USERNAME);
                        logged_in = 1; //success
                        return logged_in;
                    }
                    else {
                        printf("Fail to log in! Wrong username/password!\n");
                        logged_in = 0; //fail
                        return logged_in;
                    }
                }
            }else{
                printf("Error: Sending packet. -login\n"); 
            }
            break;
        }
        case 3: { //-addFile
            printf("Unable to use -addFile as you are NOT logged in!\n");
            logged_in = 0;
            return logged_in;
        }
        case 4: { //-deleteFile
            printf("Unable to use -deleteFile as you are NOT logged in!\n");
            logged_in = 0;
            return logged_in;
        }
        case 5: {//-fetchFile
            printf("Unable to use -fetchFile as you are NOT logged in!\n");
            logged_in = 0;
            return logged_in;
        }     
        case 6: {//-verifyFile
            printf("Unable to use -verifyFile as you are NOT logged in!\n");
            logged_in = 0;
            return logged_in;
        }
        case 7: {//-listAllFiles
            printf("Unable to use -verifyFile as you are NOT logged in!\n");
            logged_in = 0;
            return logged_in;
        }
        case 8: { //-buyCloudMoney
            printf("-BuyCloudMoney Command Received.\n");
            char *switch_data = "8"; //Tell server to switch focus to BANK
            packet_size = strlen(switch_data);
            superStringSend(sslServer, packet_size, packet_data);
            //sendDataTo(sslServer, switch_data, strlen(switch_data)); //Switch SERVER attention
            strcpy(packet_data, buyCloudMoney());
            printf("Packet Data = %s\n", packet_data);
            packet_size = strlen(packet_data);
            superStringSend(sslBank, packet_size, packet_data);
            //sendDataTo(sslBank, packet_data, 50);//Success SENDING BANK LOGIN + FUNDS
            char *amountToBuy = malloc(1000 * sizeof(char));
            strcpy(amountToBuy, getAmountToBuy());
            int sizeOfAmountToBuy = strlen(amountToBuy);
            superStringSend(sslBank, sizeOfAmountToBuy, amountToBuy);
            //sendDataTo(sslBank, amountToBuy, strlen(amountToBuy)); //Send to bank
//                char *received_packet = malloc (1000 * sizeof(char));
//                if(receiveDataFrom(sslServer, received_packet, 30000) == 1) {//Success HANDLE REPLY
//                    if(strcmp(received_packet,"1") == 0) {
//                        printf("Login successful!\n");
//                        sscanf(packet_data, "%*d %s %*s", LOGGED_IN_AS_USERNAME);
//                        logged_in = 1; //success
//                        return logged_in;
//                    }
//                }
            logged_in = 0;
            return logged_in;
            break;
        }
        case 9: { //-checkBankFunds
            printf("-CheckBankFunds Command Received.\n");
            strcpy(packet_data, checkBankFunds());
            printf("Packet Data = %s\n", packet_data);
            packet_size = strlen(packet_data);
            //if(sendDataTo(sslBank, packet_data, packet_size) == 1) { //Send checkfunds to bank
            if(superStringSend(sslBank, packet_size, packet_data) == 1) {
                char *received_packet = malloc (1000 * sizeof(char)); //Standby to receive amount
                    if(receiveDataFrom(sslBank, received_packet, 30000) == 1) { //receive amount
                        printf("You have %s of bank funds.\n", received_packet);
                    }
            }
            logged_in = 0;
            return logged_in;
            break;
        }
        case 10: { //-checkCloudFunds
            printf("Unable to use -checkCloudFunds as you are NOT logged in!\n");
            logged_in = 0;
            return logged_in;
            break;
        }
        case 11: { //-registerBankAccount
            printf("-registerBankAccount Command Received.\n");
            strcpy(packet_data, registerBankAccount());
            printf("Packet Data = %s\n", packet_data);
            packet_size = strlen(packet_data);
            //if(sendDataTo(sslBank, packet_data, packet_size) == 1) {//Success SEND
            if(superStringSend(sslBank, packet_size, packet_data) == 1) {
                char *received_packet = malloc (1000 * sizeof(char));
                if(receiveDataFrom(sslBank, received_packet, 30000) == 1) {//Success HANDLE REPLY
                    if(strcmp(received_packet,"1") == 0) {
                        printf("Bank Login successful!\n");
                        //sscanf(packet_data, "%*d %s %*s", LOGGED_IN_AS_USERNAME);
                        logged_in = 0; //success
                        return logged_in;
                    }
                    else {
                        printf("Bank Login FAIL!!\n");
                        logged_in = 0; //fail
                        return logged_in;
                    }
                }
            }else{
                printf("Error: Sending packet. -login\n"); 
            }
            break;
        }
        case 99: {//-help
            printf("%s", get_HelpList());
            break;
        }
        default: {
            logged_in = 0;
            return logged_in;
        }
    }
    return logged_in;
}
/**
 * Access to this function is only alllowed when USER is LOGGED IN
 * @param ssl ssl socket
 */
void processLoggedInUserInputs(SSL *sslServer, SSL* sslBank) {

    char *packet_data = malloc(1000 * sizeof(char));
    printf("<%s>Input Command: ", LOGGED_IN_AS_USERNAME);
    char *userCommand = malloc(100 * sizeof(char));
    scanf("%s", userCommand);
    /**
     * Used for verifying the user Command
     */
    int packet_size;
    int option = 404; //Default error
    option = verifyUserCommand(userCommand);
    if(option == ERROR) {
        printf("Invalid Command. - Please retype your command or try -help.\n");
        return;
    }
    switch (option) {
        case 0: {//-exit
           printf("%s", processExitOption());
           exit(EXIT_SUCCESS);
           break;
        }
        case 1: {
            printf("Unable to use -registerNewAccount as you are already LOGGED IN!.\n");
            break;
        }
        case 2: {
            printf("Unable to use -login as you are already LOGGED IN!.\n");
            break;
        }
        case 3: {//-addFile
            int status = 0;
            status = sendFileToServer(sslServer);
            if(status == 1) {//Successful
                printf("Your file has been added to your storage!\n");
            }
            else {
                printf("Error: Sending packet. -addFile\n");
            }
            break;
        }
        case 4: {//-deleteFile
            int status = 0;
            status = deleteFileFromServer(sslServer);
            if(status == 1) {//Successful
                printf("Your file has been deleted from your storage!\n");
            }
            else {
                printf("Error: Sending packet. -deleteFile\n");
            }
            break;
        }
        case 5: {//-fetchFile
            int status = 0;
            char *packet = fetchFile();
            status = receiveFileFromServer(sslServer, packet);
            if(status == 1) {
                printf("Your file has been fetched from storage!\n");
            }
            else{
                printf("Error: Fetching file. -fetchFile\n");
            }
            break;
        }     
        case 6: {//-verifyFile
            int status = 0;
            status = verifyFileOnServer(sslServer);
            if(status == 1) {
                printf("Your file has been verified successfully.\n");
            }
            else {
                printf("Error: Verifying file. -verifyFile\n");
            }
            break;
        }
        case 7: {//listAllFiles
            int status = 0;
            status = listAllFilesOnServer(sslServer);
            if(status == 1) {
                printf("Your files have been listed successfully.\n");
            }
            else {
                printf("Error: Listing files. -listAllFiles\n");
            }
            break;
        }
        case 8: {//-buyCloudMoney
            printf("Unable to use -buyCloudMoney as you are already LOGGED IN!.\n");
            break;
        }
        case 9: {//-checkBankFunds
            printf("Unable to use -checkBankFunds as you are already LOGGED IN!.\n");
            break;
        }
        case 10: {//-checkCloudFunds
            printf("-CheckCloudFunds Command Received.\n");
            strcpy(packet_data, checkCloudFunds());
            printf("Packet Data = %s\n", packet_data);
            packet_size = strlen(packet_data);
            //if(sendDataTo(sslServer, packet_data, packet_size) == 1) { //Send checkfunds to bank
            if(superStringSend(sslServer, packet_size, packet_data) == 1) {
                char *received_packet = malloc (1000 * sizeof(char)); //Standby to receive amount
                    if(receiveDataFrom(sslServer, received_packet, 30000) == 1) { //receive amount
                        printf("You have %s of cloud funds.\n", received_packet);
                    }
            }
            break;
        }
        case 99: {//-help for logged in
            printf("%s", get_HelpList2());
            break;
        }
    }
}
SSL* connectToCloudServer(char *strings[]) {
    SSL_CTX *ctx;  
    int server;  
    SSL *ssl;  
    char buf[1024];  
    int bytes;  
    char *hostname, *portnum;
    
    hostname=strings[1];  // SERVER ADDRESS
    portnum=strings[2]; // SERVER PORT
    
    ctx = InitCTX();  
    server = OpenConnection(hostname, atoi(portnum));  
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    BIO* sslserver = BIO_new_socket(server, BIO_NOCLOSE);
    //BIO_set_nbio(sslserver,0);
    SSL_set_bio(ssl, sslserver, sslserver);
   //SSL_set_fd(ssl, server);    /* attach the socket descriptor */  
    if ( SSL_connect(ssl) == FAIL ) {  /* perform the connection */  
        ERR_print_errors_fp(stderr);
    }
    else {
            ShowServerCerts(ssl);        /* get any certs */

    }
    return ssl;
}
SSL* connectToCloudBank(char *strings[]) {
    SSL_CTX *ctx;  
    int server;  
    SSL *ssl;  
    char buf[1024];  
    int bytes;  
    char *hostname, *portnum;
    
    hostname=strings[3]; //BANK ADDRESS  
    portnum=strings[4]; //BANK PORT
    printf("Hostname: %s | Port: %s\n", hostname, portnum);
    
    ctx = InitCTX();  
    server = OpenConnection(hostname, atoi(portnum));  
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    BIO* sslserver = BIO_new_socket(server, BIO_NOCLOSE);
    //BIO_set_nbio(sslserver,0);
    SSL_set_bio(ssl, sslserver, sslserver);
   //SSL_set_fd(ssl, server);    /* attach the socket descriptor */  
    if ( SSL_connect(ssl) == FAIL ) {  /* perform the connection */
        printf("FAIL TO CONNECT\n");
        ERR_print_errors_fp(stderr);
        printf("DONE1!!!\n");
    }
    else {
            ShowBankCerts(ssl);        /* get any certs */

    }
    return ssl;
  
}
/**
 * Main execution
 * @param count - IP Address
 * @param strings - Port number
 * @return 
 */
int main(int count, char *strings[]) { 
    if ( count != 5 )  {  
        printf("usage: %s <server_hostname> <server_portnum> <bank_hostname> <bank_portnum>\n", strings[0]);  
        exit(0);  
    }
    SSL_library_init(); 
    SSL *sslServer = connectToCloudServer(strings);
    //printf("Created SSLSERVER.\n");
    SSL *sslBank = connectToCloudBank(strings);
    //printf("Created SSLCLOUDBANK\n");
    system("clear");
    
    /*Beginning of actual calling*/
    printf("%s\n", get_IntroMsg());
    int logged_in = 0; //Not logged in
    int status = 0;
    while (1) {
        if(logged_in == 0) {
            logged_in = processCommonInputs(sslServer, sslBank); //On succesful login, return 1
            if(logged_in == 1) { //Only called once
                system("clear");
                printf("Thanks for logging in!\nYou are now able to access the full cloud commands!.\n\nLogged in as: %s\n\n", LOGGED_IN_AS_USERNAME);
                logged_in = 2; 
            }
        } else {
            processLoggedInUserInputs(sslServer,sslBank);
        }
                    
    }
}  
//    close(server);         /* close socket */  
//    SSL_CTX_free(ctx);        /* release context */  
//    return 0;  
 