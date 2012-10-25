/* 
 * File:   bank.c
 * Author: MarcusDerek
 *
 * Created on October 25, 2012, 11:17 PM
 */

//SSL-Bank.c 

#include <stdio.h>  
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

#define FAIL    -1
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
/*
 * 
 */
void serviceConnection(SSL *ssl) {
    char *file_packet = malloc(1000 * sizeof(char));  
    char *reply = "Success!";  
    int sd, bytes;  
   
    //ShowClientCerts(ssl);        /* get any certificates */
    while(1) {
        receiveDataFrom(ssl, file_packet, 1000);
        printf("Bank received: %s\n", file_packet);
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
        serviceConnection(sslServer);         /* service connection. 1 user 1 persistant connection */  
    } 
    //close(server);          /* close server socket */  
    //SSL_CTX_free(ctx);         /* release context */  
}

