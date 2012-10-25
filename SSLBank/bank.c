/* 
 * File:   bank.c
 * Author: MarcusDerek
 *
 * Created on October 25, 2012, 11:17 PM
 */

//SSL-Bank.c 

//#include "openssl/include/openssl/bio.h"
//#include "openssl/include/openssl/ssl.h"
//#include "openssl/include/openssl/err.h"

#include <errno.h>  
#include <unistd.h>  
#include <string.h>  
#include <arpa/inet.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <resolv.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

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

/*
 * 
 */
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

