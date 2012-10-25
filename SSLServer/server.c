/* 
 * File:   main.c
 * Author: MarcusDerek
 * SERVER
 *
 * Created on October 23, 2012, 6:36 PM
 */

//SSL-Server.c  
#include <errno.h>  
#include <unistd.h>  
#include <string.h>  
#include <arpa/inet.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <resolv.h> 

#include "openssl/include/openssl/bio.h"
#include "openssl/include/openssl/ssl.h"
#include "openssl/include/openssl/err.h"

#define CLOUD_SERVER_PORT "3230"  // the port users will be connecting to
#define CLOUD_SERVER_ADDRESS "130.95.1.70"
#define TESTPORT "9999" //Port for simulating cloud server
#define BACKLOG 10     // how many pending connections queue will hold
#define FILESERVER_AREA "/Users/MarcusDerek/Documents/UWA/Storage"
#define MAX_FILE_SIZE 50000
#define FAIL    -1

/*Global Variable*/
char LOGGED_IN_AS_USERNAME[100];
int LOGGED_IN = 0; //1 logged in, 0 otherwise. Default: NOT LOGGEDIN

/* -------------- BASE CODE - DO NOT TOUCH *----------------------------------------------------------*/  
int OpenListener(int port) {
    int sd;  
    struct sockaddr_in addr;  
  
    sd = socket(PF_INET, SOCK_STREAM, 0);  
    bzero(&addr, sizeof(addr));  
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(port);  
    addr.sin_addr.s_addr = INADDR_ANY;  
    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 ) {  
        perror("can't bind port");  
        abort();  
    }  
    if ( listen(sd, 10) != 0 ) {  
        perror("Can't configure listening port");  
        abort();  
    }  
    return sd;  
}  
  
SSL_CTX* InitServerCTX(void)  {
    SSL_METHOD *method;  
    SSL_CTX *ctx;  
  
    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */  
    SSL_load_error_strings();   /* load all error messages */  
    method = SSLv2_server_method();  /* create new server-method instance */  
    ctx = SSL_CTX_new(method);   /* create new context from method */  
    if ( ctx == NULL )  {  
        ERR_print_errors_fp(stderr);  
        abort();  
    }  
    return ctx;  
}  
/**
 * Generates Private Key using Keyfile
 * @param ctx
 * @param CertFile
 * @param KeyFile
 */  
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)  {  
 /* set the local certificate from CertFile */  
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )  {  
        ERR_print_errors_fp(stderr);  
        abort();  
    }  
    /* set the private key from KeyFile (may be the same as CertFile) */  
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )  
    {  
        ERR_print_errors_fp(stderr);  
        abort();  
    }  
    /* verify private key */  
    if ( !SSL_CTX_check_private_key(ctx) )  
    {  
        fprintf(stderr, "Private key does not match the public certificate\n");  
        abort();  
    }  
}  
 
void ShowCerts(SSL* ssl)  
{   X509 *cert;  
    char *line;  
  
    cert = SSL_get_peer_certificate(ssl); /* Get Client certificates (if available as not necessary) */  
    if ( cert != NULL )  
    {  
        printf("Server certificates:\n");  
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);  
        printf("Subject: %s\n", line);  
        free(line);  
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);  
        printf("Issuer: %s\n", line);  
        free(line);  
        X509_free(cert);  
    }  
    else  
        printf("No client certificates available.\n");  
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
    strcat(storageFilePath,"Storage/"); 
    strcat(storageFilePath,fileName);
    pFile = fopen(storageFilePath, "wb");
    fwrite(packet, 1, size, pFile);
    fclose(pFile);
    printf("File created.\n");
}
/**
 * Receive Data from Cloud. Use for files
 * @param ssl - ssl socket
 * @param received_packet - packet data
 * @param received_packet_size - size of packet
 * @return 1 success, 0 otherwise
 */
int receiveDataFrom(SSL *ssl, char* received_packet, int received_packet_size) {
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
 * Receive the file name and size used to receive actual file
 * @param sockfd
 * @param received_packet
 * @return 
 */
int receiveFileNameSizeFrom(SSL *ssl, char *received_packet) {
    SSL_read(ssl, received_packet, 30);
}
/** 
 * @param sockfd
 * @param received_packet
 * @return 
 */
int receiveCommandFrom(SSL *ssl, char* received_packet) {
    int success = 1;
    int bytes_received = SSL_read(ssl, received_packet, 30);
    if(bytes_received == 0) {
        printf("Connection terminated abruptly.\n");
        exit(EXIT_FAILURE);
    }
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
 * @param ssl - SSL socket
 * @param sent_packet - packet data
 * @param sent_packet_size - size of data
 * @return 1 success, 0 otherwise
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
    printf("Server: Packet sent successful.\n");
    return success;
}
/**
 * Sends the File size and name to the Server
 * @param ssl
 * 
 */
void sendFileSizeNameDataTo(SSL *ssl, char *sent_packet) {
    
    int bytes_sent = SSL_write(ssl, sent_packet,30);
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
 * Register new Account
 * @param received_packet - packet data
 * @return 1 success, 0 otherwise
 */
int processRegisterNewAccount(char* received_packet) {
    char *userName = malloc(100 * sizeof(char));
    char *password = malloc(100 * sizeof(char));
    sscanf(received_packet, "%*d %s %s", userName, password);
    printf("Registering user - %s\n", userName);
    createUserHomeDirectory(userName); //BRYAN - return pass or fail
    
    int pass = 1;
    int fail = 0;
    return pass; //NEED TO CHANGE
}
/**
 * Ensure that user is correctly logged in
 * Sets GLOBAL param to ACTIVE
 * @param received_packet
 * @param ssl
 * @return 
 */
int loginToAccount(char *received_packet, SSL *ssl) {
    char *username = malloc(1000 * sizeof(char));
    sscanf(received_packet, "%*d %s %*s", username);
    strcpy(LOGGED_IN_AS_USERNAME, username); //SETS THE GLOBAL USERNAME
    LOGGED_IN = 1;
    return 1;
}
/**
 * Add files to the cloud server
 * @param ssl - ssl socket
 * @return 1 success, 0 otherwise
 */
int addFileToCloud(SSL *ssl) {
    int success = 0;
    printf("Prepared to receive FILESIZE & Name.\n");
    char *fileNameSize = malloc(1000 * sizeof(char));
    int file_size;
    int bytes_received;
    receiveFileNameSizeFrom(ssl, fileNameSize);
    char *fileName = malloc(1000 * sizeof(char));
    extractFileNameSize(fileNameSize, &file_size, fileName);
    printf("Filename: %s | Size: %d\n",fileName, file_size);
    char *file_packet = malloc(file_size * sizeof(char));
    if(receiveDataFrom(ssl, file_packet, file_size) == 1) { //RECEIVING ACTUAL FILE DATA
        packetToFile(file_packet, file_size, fileName);
        success = 1;
    } else {
        printf("Error occurred: addFileToCloud()\n");
        exit(EXIT_FAILURE);
    }
    return success;
}
/**
 * Delete the requested file from the cloud storage
 * @param received_packet - file name
 * @return 
 */
int deleteFileFromCloud(char *received_packet) {
    char *fileName = malloc(1000 * sizeof(char));
    sscanf(received_packet, "%*d %s %*s", fileName);
    //TO DO -- ADD BRYAN DELETE FILE HERE
    printf("Delete file: %s\n", fileName);
}
/**
 * Verify the file on the cloud BASED on the filename
 * @param received_packet - packet containing the filename
 * @return 
 */
int verifyFileOnCloud(char *received_packet) {
    char *fileName = malloc(1000 * sizeof(char));
    sscanf(received_packet, "%*d %s %*s", fileName);
    //TO DO -- ADD BRYAN VERIFY FILE HERE
    printf("Verifying file: %s\n", fileName);
    return 1; //TEMP
}
/**
 * Fetch the requested file from the cloud storage
 * @param received_packet - filename
 * @return 
 */
int fetchFileFromCloud(SSL *ssl, char *received_packet) {
    char *fileName = malloc(1000 * sizeof(char));
    sscanf(received_packet, "%*d %s %*s", fileName);
    char *filePath = malloc(1000 * sizeof(char));
    //sprintf(filePath, "Storage/%s/%s", LOGGED_IN_AS_USERNAME, fileName); //UNCOMMENT WITH REAL ACCOUNT
    sprintf(filePath, "Storage/%s", fileName);
    size_t file_size;
    char *fileBuffer = compress_File_to_stream(&file_size, filePath); //Specify filepath and retrieve filesize
    char *fileSizeName = malloc(1000 * sizeof(char));
    sprintf(fileSizeName,"0 %lu %s", file_size, fileName);
    sendFileSizeNameDataTo(ssl, fileSizeName); //Send Filesize and name back to the CLIENT
    printf("FileNameSize: %s\n", fileSizeName);
    if(send_all_data(ssl, fileBuffer, file_size) == 0) { //Success
        printf("Fetch success!\n");
        return 1;
    }
    else {
        printf("Fetch fail!\n");
        return 0;
    }
}
int listAllFilesOnCloud() {
    //RETURN LIST OF FILES
    //char *listOfFiles = returnListofFiles();
//    if(send_all_data(ssl, listOfFiles, strlen(listOfFiles)) == 0) { //Success
//        printf("Fetch success!\n");
//        return 1;
//    }
//    else {
//        printf("Fetch fail!\n");
//        return 0;
//    }
    return 1;// TEMP
}

/** 
 * Main bulk of servicing the incomming connection
 * @param ssl
 */
void serviceConnection(SSL* ssl) /* Serve the connection -- threadable */  
{   char buf[1024];  
    char *reply = "Success!";  
    int sd, bytes;  
  
    if ( SSL_accept(ssl) == FAIL )  {
        /* do SSL-protocol accept */  
        ERR_print_errors_fp(stderr);
        printf("Error OCCURED!\n");
    }   
    else {   
        ShowCerts(ssl);        /* get any certificates */  
        while(1) {
            char *received_packet = malloc(1000 * sizeof(char));
            int file_size;
            char *sent_packet = malloc(1000 * sizeof(char));

            if(receiveCommandFrom(ssl, received_packet) == 1) {
                printf("Received MSG: %s\n", received_packet);
            } else {
                printf("Error: Receive invalid command!\n");
                exit(EXIT_FAILURE);
            }
            int userInputCommand;
             /*
             * Determine which command is used
             */
            
            int status;
            sscanf(received_packet, "%d", &userInputCommand);
            /**
            * This section is used to validate the commands
            */
            switch (userInputCommand) {
                case 1: { //-registerNewAccount
                    status = processRegisterNewAccount(received_packet); //ie 1 Marcus 123456 - 1 = registerNewAccount
                    if(status == 1) { //1 == Pass, 0 == fail
                        sendDataTo(ssl, "1", 10);
                        printf("-registerNewAccount: Success\n");
                    } else {
                        sendDataTo(ssl, "0", 10);
                        printf("-registerNewAccount: Fail\n");
                    }
                    break;
                }
                case 2: { //-login
                    status = loginToAccount(received_packet, ssl);//ie 2 Marcus 123456
                    if(status == 1) { //1 == Pass, 0 == fail
                        sendDataTo(ssl, "1", 10);
                        printf("-login: Success\n");
                    } else {
                        sendDataTo(ssl, "0", 10);
                        printf("-login: Fail\n");
                    }
                    break;
                }
                case 3: { //-addFile
                    status = addFileToCloud(ssl);
                    if(status == 1) { //1 == Pass, 0 == fail
                        sendDataTo(ssl, "1", 10);
                        printf("-addFile: Success\n");
                    } else {
                        sendDataTo(ssl, "0", 10);
                        printf("-addFile: Fail.\n");
                    }
                    break;
                }
                case 4: { //-deleteFile
                    status = deleteFileFromCloud(received_packet);
                    if(status == 1) { //1 == Pass, 0 == fail
                        sendDataTo(ssl, "1", 10);
                        printf("-addFile: Success\n");
                    } else {
                        sendDataTo(ssl, "0", 10);
                        printf("-addFile: Fail.\n");
                    }
                    break;
                }
                case 5: {//-fetchFile
                    status = fetchFileFromCloud(ssl, received_packet);
                    if(status == 1) {
                        printf("-fetchFile: Success\n");
                    }
                    else {
                        printf("-fetchFile: Fail.\n");
                    }
                    break;
                }
                case 6: {//-verifyFile
                    status = verifyFileOnCloud(received_packet);
                    if(status == 1) {
                        sendDataTo(ssl, "1", 10);
                        printf("-verifyFile: Success\n");
                    }
                    else {
                        sendDataTo(ssl, "0", 10);
                        printf("-verifyFile: Fail.\n");
                    }
                    break;
                }
                case 7: {//-listAllFiles
                    status = listAllFilesOnCloud();
                }
                    
            }
        }

        
    }  
    //sd = SSL_get_fd(ssl);       /* get socket connection */  
    //SSL_free(ssl);         /* release SSL state */  
    //close(sd);          /* close connection */  
}  


int main(int count, char *strings[])  
{   SSL_CTX *ctx;  
    int server;  
    char *portnum;  
  
    if ( count != 2 )  
    {  
        printf("Usage: %s <portnum>\n", strings[0]);  
        exit(0);  
    }
    system("clear");
    SSL_library_init(); //Required to startup SSL 
  
    portnum = strings[1];  
    ctx = InitServerCTX();        /* initialize SSL */  
    LoadCertificates(ctx, "mycert.pem", "mycert.pem"); /* load certs from server directory */  
    server = OpenListener(atoi(portnum));    /* create server socket */  
    struct sockaddr_in addr;  
    socklen_t len = sizeof(addr);  
    SSL *ssl;  
    /* Server Initation */
    printf("Server Status: Online\nWaiting for connection...\n");
    int client = accept(server, (struct sockaddr*)&addr, &len);  /* accept connection as usual */  
    printf("Connection received: %s:%d\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));  
    ssl = SSL_new(ctx);              /* get new SSL state with context */
    BIO* sslclient = BIO_new_socket(client, BIO_NOCLOSE);
    SSL_set_bio(ssl, sslclient, sslclient);   
    printf("Parameters created.\nAwaiting commands...\n");
    /* Put all processes in the WHILE LOOP */
    while(1) {
        serviceConnection(ssl);         /* service connection. 1 user 1 persistant connection */  
    } 
    close(server);          /* close server socket */  
    SSL_CTX_free(ctx);         /* release context */  
}  