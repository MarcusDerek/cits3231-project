/* 
 * File:   bank.c
 * 
 * Team: 
 * Marcus Derek - 11016403
 * Taiga Yano - 20698782
 * Bryan Kho - 20714477
 *
 */

//SSL-Bank.c 

#include <stdio.h>  
#include <stdlib.h>
#include <errno.h>  
#include <unistd.h>    
#include <resolv.h>  
#include <netdb.h> 
#include <string.h>  
#include <arpa/inet.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdbool.h>
#include <openssl/sha.h>

#define FAIL    -1
#define SALT "banksalt"

/*Global Variable*/
char LOGGED_IN_AS_USERNAME[100];
int LOGGED_IN = 0; //1 logged in, 0 otherwise. Default: NOT LOGGEDIN
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
 * @param sockfd
 * @param received_packet
 * @return 
 */
int receiveCommandFrom(SSL *ssl, char* received_packet) {
    int success = 1;
    int bytes_received = SSL_read(ssl, received_packet, 50);
    if(bytes_received == 0) {
        printf("Connection terminated abruptly.\n");
        exit(EXIT_FAILURE);
    }
    return success;
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
    else {
        printf("No certificates. Exiting program.\n");
        exit(EXIT_FAILURE); //Exits when the server does not have a cert
    }  
        
}
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
 
void ShowServerCerts(SSL* ssl)  
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
SSL* acceptClientConnection(int count, char *strings[]) {
    SSL_CTX *ctx;  
    int server;  
    char *portnum;  

    //system("clear");
    SSL_library_init(); //Required to startup SSL 
  
    portnum = strings[1]; //Client Port  
    ctx = InitServerCTX();        /* initialize SSL */  
    LoadCertificates(ctx, "mycert.pem", "mycert.pem"); /* load certs from server directory */  
    server = OpenListener(atoi(portnum));    /* create server socket */  
    struct sockaddr_in addr;  
    socklen_t len = sizeof(addr);  
    SSL *ssl;  
    /* Server Initation */
    printf("Server Status: Online\nCloud Server fully operational.\nWaiting for connection (CLIENT) on port: %s...\n",portnum );
    int client = accept(server, (struct sockaddr*)&addr, &len);  /* accept connection as usual */  
    printf("Connection received (CLIENT): %s:%d\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));  
    ssl = SSL_new(ctx);              /* get new SSL state with context */
    BIO* sslclient = BIO_new_socket(client, BIO_NOCLOSE);
    SSL_set_bio(ssl, sslclient, sslclient);   
    printf("Parameters created.\nAwaiting commands...\n");
    return ssl;
}
SSL* connectToCloudServer(char *strings[]) {
    SSL_CTX *ctx;  
    int server;  
    SSL *ssl;  
    char buf[1024];  
    int bytes;  
    char *hostname, *portnum;
    
    hostname=strings[2];  // SERVER ADDRESS
    portnum=strings[3]; // SERVER PORT
    
    ctx = InitCTX();  
    server = OpenConnection(hostname, atoi(portnum));  
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    BIO* sslserver = BIO_new_socket(server, BIO_NOCLOSE);
    //BIO_set_nbio(sslserver,0);
    SSL_set_bio(ssl, sslserver, sslserver);
   //SSL_set_fd(ssl, server);    /* attach the socket descriptor */
    int statusServer;
    if (SSL_connect(ssl) <= 0 ) {  /* perform the connection */  
        ERR_print_errors_fp(stderr);
        printf("Server failed to connect!!!\n");
        exit(EXIT_FAILURE);
    }
    else {
            //ShowCerts(ssl);        /* get any certs */
            if(SSL_get_verify_result(ssl) != X509_V_OK) {
                printf("Fail verification. NEED TO WORK ON THIS!!!!\n");
                //exit(EXIT_FAILURE);
            }
    }
    printf("Bank: Connected to CloudServer.\n");
    printf("Server Status: %d\n", statusServer);
    return ssl;
}

/**
 * WORKING!
 * @param input
 * @return 
 */
char *generate_hash(char *input){
    char *salted = malloc((strlen(input)+1000) * sizeof(char));
    strcat(salted,input);
    strcat(salted,SALT);
    unsigned char hash[10];
    SHA1(salted,strlen(salted),hash);
    unsigned char *final = malloc((strlen(hash)+1000) * sizeof(unsigned char));
    int i;
    for (i = 0; i < strlen(hash);i++){
        sprintf(final,"%s%u",final,hash[i]);
    }
    return final;
}

int create_account(char *received_packet, SSL *sslClient){
    int found = 0;
    char* username = malloc(1000 * sizeof(char));
    char* password = malloc(1000 * sizeof(char));
    sscanf(received_packet, "%*d %s %s", username, password);
    
    FILE *fp;
    
    if((fp = fopen("password_file.txt","r+")) == NULL) {//need to append{
        printf("Error: Opening file.\n");
        exit(EXIT_FAILURE);
    } 
    char* line = malloc(1000 * sizeof(char));
    char* username2 = malloc(1000* sizeof(char));
    char* password2 = malloc(1000* sizeof(char));
    while(!feof(fp)) {
       
        fgets(line, 1000, fp);
        printf("Scanning: %s\n", line);

        sscanf(line,"%s %s %*s",username2,password2);
        if(strcmp(username2,username) == 0) { //If found.. quit! Bryan say so!
            found = 1;
            break;
        }
        
    }
    if(found == 1) {
        printf("User already exist in system!.\n\n");
        fclose(fp);
        return 0;
    }
    else {
        printf("Creating new user %s, password %s.\n",username,password);
        char *buffer = malloc (1000 * sizeof (char));
        sprintf(buffer,"%s %s 0\n", username, generate_hash(password));
        fputs(buffer,fp);
        fclose(fp);
        return 1;
    }
}


bool withdraw(char *account, char *amount){
	FILE *fp;
	
	if ((fp = fopen("password_file.file","r+"))==NULL) {
		//error
	}
	char *line = malloc(256 * sizeof(char));
	bool found = false;
	while(1){
		if (feof(fp)||ferror(fp)) {
			break;
		}
		fgets(line, sizeof(line), fp);
		char *email_in_list = malloc(512 * sizeof(char));
                sscanf(line,"%s %*s",email_in_list);
		if(strcmp(email_in_list,account) == 0){
			char *user = malloc(128 * sizeof(char));
			char *hash = malloc(256 * sizeof(char));
			char *balance = malloc(128 * sizeof(char));
			sscanf("%s %s %s",user,hash,balance);
			
			if(atoi(balance) - atoi(amount) < 0){
				printf("not enough balance!\n");
			}
			else {
				sprintf(balance,"%d",(atoi(balance) - atoi(amount)));
				sprintf(line,"%s %s %s",user, hash, balance);
			}

			return true;
		}
	}
	return false;
	

}
char *check_balance(char *account){
	FILE *fp;
	
	if ((fp = fopen("password_file.file","rw"))==NULL) {
		//error
	}
	char *line = malloc(256 * sizeof(char));
	bool found = false;
	while(1){
		if (feof(fp)||ferror(fp)) {
			break;
		}
		fgets(line, sizeof(line), fp);
		char *email_in_list = malloc(512 * sizeof(char));
                sscanf(line,"%s %*s",email_in_list);
		if(strcmp(email_in_list,account) == 0){
                    return line;
                }
        }
        return "unable to retrieve the balance";
	
}



bool compare_hash(char *hash_alpha, char *hash_bravo){
    return strcmp(hash_alpha, hash_bravo);
}

/**
 * Ensure that user is correctly logged in
 * Sets GLOBAL param to ACTIVE
 * @param received_packet
 * @param ssl
 * @return 
 */
int loginToBankAccount(char *received_packet, SSL *ssl) {
    int found;
    char *username = malloc(1000 * sizeof(char));
    char *password = malloc(1000 * sizeof(char));
    sscanf(received_packet, "%*d %s %s", username,password);
    FILE *fp;
    if((fp = fopen("password_file.txt","r"))==NULL){
        printf("Error reading the password file\n");
        exit(EXIT_FAILURE);
    }
    char *hashedPassword = generate_hash(password);
    char* username2 = malloc(1000* sizeof(char));
    char* password2 = malloc(1000* sizeof(char));
    char* line = malloc(1000 * sizeof(char));
    while(!feof(fp)) {
        
        fgets(line, 1000, fp);
        
        sscanf(line,"%s %s %*s",username2,password2);
        printf("\n%s\n%s\n",hashedPassword , password2);
        if(strcmp(username2,username) == 0) { //If found.. quit! Bryan say so!
            if(strcmp(hashedPassword,password2) == 0) { //Success
                strcpy(LOGGED_IN_AS_USERNAME, username); //SETS THE GLOBAL USERNAME
                LOGGED_IN = 1;
                return 1; //success
            }
        }
    }
    return 0;


}
int addCloudMoneyToServer(SSL *sslServer, char *cloudMoney) {
    //char *cloudMonies = malloc(1000 * sizeof(char));
    //sprintf(cloudMonies, "%d", cloudMoney);
    sendDataTo(sslServer, cloudMoney, strlen(cloudMoney));
}
/*
 * 
 */
void serviceConnection(SSL *sslClient, SSL *sslServer) {
    char *file_packet = malloc(1000 * sizeof(char));  
    char *reply = "Success!";  
    int sd, bytes;  
    
    while(1) {
    char *received_packet = malloc(1000 * sizeof(char));
    char modifiedPacket[50];
    int file_size;
    char *sent_packet = malloc(1000 * sizeof(char));
    //ShowClientCerts(ssl);        /* get any certificates */
    
            if(receiveCommandFrom(sslClient, received_packet) == 1) {
                strncpy(modifiedPacket, received_packet, sizeof(modifiedPacket));
                printf("Received MSG: %s\n", modifiedPacket);
            } else {
                 printf("Error: Receive invalid command!\n");
                //exit(EXIT_FAILURE);
            }
    int userInputCommand;
     /*
     * Determine which command is used
     */
    int status;
    sscanf(received_packet, "%d", &userInputCommand);
        switch (userInputCommand) {
            case 8: { //-buyCloudMoney
                status = loginToBankAccount(received_packet, sslClient);//ie 8 Marcus 123456
                if(status == 1) { //1 == Pass, 0 == fail
                    char *amountToBuy = malloc(1000 * sizeof(char));
                    receiveCommandFrom(sslClient, amountToBuy);
                    printf("Amount to buy = %s\n", amountToBuy);
                    //TAIGA - TO DEDUCT MONEY FROM FILE + cloudMoney RETURN FAIL OR SUCCESS
                    if(withdraw(LOGGED_IN_AS_USERNAME,amountToBuy) == false){
                        printf("aborting\n");
                    }
                    //MARCUS - TO DO SUCCES OR FAIL STATEMENT
                    printf("Withdraw successful.\n");
                    int statusMoney = addCloudMoneyToServer(sslServer, amountToBuy);
                    //sendDataTo(sslClient, "1", 10);
                    //printf("-login: Success\n");
                } else {
                    printf("Error logging into bank!!\n");
                    //sendDataTo(sslClient, "0", 10);
                    //printf("-login: Fail\n");
                }
                break;
            }
            case 9: {
                status = loginToBankAccount(received_packet, sslClient); // 9 Marcus 123456
                if(status == 1) {//Success 
                    //TAIGA - CHECK FUNDS IN BANK FILE AND GRAB IT
                    char *FUNDS = check_balance(LOGGED_IN_AS_USERNAME);
                    sendDataTo(sslClient, FUNDS, 10);
                }
                break;
            }
            case 11: { //createAccoutn
                status = create_account(received_packet, sslClient);
                if(status == 1) { //1 == Pass, 0 == fail
                    sendDataTo(sslClient, "1", 10);
                    printf("-registerBankAccount: Success\n");
                } else {
                    sendDataTo(sslClient, "0", 10);
                    printf("-registerBankAccount: Fail\n");
                }
                break;
                
            }
        }
    }

}
int main(int count, char *strings[])  {
    if ( count != 4 ) {  
        printf("Usage: %s <client_portnum> <server_address> <server_port> \n", strings[0]);  
        exit(0);  
    }
    SSL_library_init();
    SSL *sslServer = connectToCloudServer(strings);
    SSL *sslClient = acceptClientConnection(count, strings);
    int statusClient = SSL_accept(sslClient);
    printf("Bank: StatusClient = %d\n", statusClient);
    
    if(statusClient == FAIL) {
        ERR_print_errors_fp(stderr);
        printf("Error: Accepting Client connection.\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        serviceConnection(sslClient, sslServer);         /* service connection. 1 user 1 persistant connection */  
    } 
    //close(server);          /* close server socket */  
    //SSL_CTX_free(ctx);         /* release context */  
}

