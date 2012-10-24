/* 
 * File:   main.c
 * Author: MarcusDerek
 * CLIENT
 *
 * Created on October 23, 2012, 6:36 PM
 */

#include "openssl/include/openssl/bio.h"
#include "openssl/include/openssl/ssl.h"
#include "openssl/include/openssl/err.h"

//SSL-Client.c  
#include <stdio.h>  
#include <errno.h>  
#include <unistd.h>   
#include <string.h>  
#include <sys/socket.h>  
#include <resolv.h>  
#include <netdb.h>
#include "clientCommands.h"
  
#define CLOUD_SERVER_PORT "9999"
#define CLOUD_SERVER_ADDRESS "192.168.211.130"
#define DEFAULT_SIZE 100
#define ERROR 404
#define MAX_FILE_SIZE 50000
#define FAIL    -1  

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
  
void ShowCerts(SSL* ssl)  {
    X509 *cert;  
    char *line;  
  
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */  
    if ( cert != NULL )  {  
        printf("Server certificates:\n");  
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);  
        printf("Subject: %s\n", line);  
        free(line);       /* free the malloc'ed string */  
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);  
        printf("Issuer: %s\n", line);  
        free(line);       /* free the malloc'ed string */  
        X509_free(cert);     /* free the malloc'ed certificate copy */  
    }  
    else  
        printf("No certificates.\n");
    //exit(EXIT_FAILURE);
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
    printf("Client: Packet sent successful.\n");
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
        printf("%d bytes sent to client.\n", n);
        if (n == -1) { 
            printf("BREAK!!!!\n");
            break; 
        }
        bytes_sent = bytes_sent + n;
        bytes_remaining = bytes_remaining - n;
    }
	printf("Total bytes sent = %d\n", bytes_sent);

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
    printf("Client: Packet received successful.\n");
    return success;
    
}
void processUserInputs(SSL *ssl) {
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
        return;
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
            if(sendDataTo(ssl, packet_data, packet_size) == 1) {//Success SEND
                char *received_packet = malloc (1000 * sizeof(char));
                if(receiveDataFrom(ssl, received_packet, 30000) == 1) {//Success RECEIVE REPLY
                    if(strcmp(received_packet,"1") == 0) {
                        printf("Account registration successful!\n");
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
            if(sendDataTo(ssl, packet_data, packet_size) == 1) {//Success SEND
                char *received_packet = malloc (1000 * sizeof(char));
                if(receiveDataFrom(ssl, received_packet, 30000) == 1) {//Success HANDLE REPLY
                    if(strcmp(received_packet,"1") == 0) {
                        printf("Login successful!\n");
                    }
                }
            }else{
                printf("Error: Sending packet. -login\n"); 
            }
            break;
        }
        case 3: {//-addFile
            sendDataTo(ssl, "3", 5); //Trigger server to receive file
            char *filePath = malloc(1000 * sizeof(char));
            size_t file_size;
            strcpy(filePath, addFile());
            extractFileName(filePath);
            char *fileBuffer = compress_File_to_stream(&file_size, filePath);
            char *convertFileSize = malloc(100 *sizeof(char));
            sprintf(convertFileSize,"%lu", file_size);
            char *fileNameSize = concatSentence(0, convertFileSize, extractFileName(filePath)); //0 = null
            sendFileSizeNameDataTo(sockfd, fileNameSize);
            if(send_all_data(sockfd, fileBuffer, file_size) == 0) { //Success
                char *received_packet = malloc (1000 * sizeof(char));
                if(receiveDataFrom(sockfd, received_packet, 30000) == 1) {//Success HANDLE REPLY
                    if(strcmp(received_packet,"1") == 0) {
                        printf("Your file has been added to your storage!\n");
                    }
                }
            }else{
                printf("Error: Sending packet. -addFile\n"); 
            }
            break;
        }
        case 4: {
            strcpy(packet_data, deleteFile()); //Return filename to be deleted
            packet_size = strlen(packet_data);
            if(sendDataTo(sockfd, packet_data, packet_size) == 1) { //Success SEND
                char *received_packet = malloc (1000 * sizeof(char));
                if(receiveDataFrom(sockfd, received_packet, 30000) == 1) {//Success HANDLE REPLY
                    if(strcmp(received_packet,"1") == 0) {
                        printf("Your file has been deleted from storage!\n");
                    }
                }
            }else{
                printf("Error: Sending packet. -addFile\n"); 
            }
            break;
        }
        case 5: {//-fetchFile
            
            break;
        }     
        case 6: //-verifyFile
            break;      
    }
}
/**
 * Main execution
 * @param count - IP Address
 * @param strings - Port number
 * @return 
 */
int main(int count, char *strings[])  
{   SSL_CTX *ctx;  
    int server;  
    SSL *ssl;  
    char buf[1024];  
    int bytes;  
    char *hostname, *portnum;  
  
    if ( count != 3 )  {  
        printf("usage: %s <hostname> <portnum>\n", strings[0]);  
        exit(0);  
    }  
    SSL_library_init();  
    hostname=strings[1];  
    portnum=strings[2];  
  
    ctx = InitCTX();  
    server = OpenConnection(hostname, atoi(portnum));  
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    BIO* sslserver = BIO_new_socket(server, BIO_NOCLOSE);
    //BIO_set_nbio(sslserver,0);
    SSL_set_bio(ssl, sslserver, sslserver);
   //SSL_set_fd(ssl, server);    /* attach the socket descriptor */  
    if ( SSL_connect(ssl) == FAIL )   /* perform the connection */  
        ERR_print_errors_fp(stderr);  
    else  {
        ShowCerts(ssl);        /* get any certs */
        while (1) {
            processUserInputs();
//            printf("Connected with %s encryption\n", SSL_get_cipher(ssl)); //Mandatory
//            printf("Input Command: ");
//            char *userCommand = malloc(100 * sizeof(char));
//            scanf("%s", userCommand);
//            SSL_write(ssl, userCommand, strlen(userCommand));   /* encrypt & send message */ 
//            char *receive_reply = malloc(1000 * sizeof(char));
//            bytes = SSL_read(ssl, receive_reply, 9); /* get reply & decrypt */
//            printf("Received: %s\n", receive_reply);
//            free(receive_reply);
//            //SSL_free(ssl);        /* release connection state */ 
        }
    }  
    close(server);         /* close socket */  
    SSL_CTX_free(ctx);        /* release context */  
    return 0;  
}  