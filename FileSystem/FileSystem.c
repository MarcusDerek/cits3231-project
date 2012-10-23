//
//  FileSystem.c
//  CloudClient
//
//  Created by Bryan Kho on 14/10/2012.
//  Copyright (c) 2012 . All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
//#include <config.h>
//#include <autoconf.h>
#include "FileSystem.h"

/*******
 NOTES : MARCUS & TAIGA
 - Make sure change the PATH TO ACCOMODATE YOUR OS as I have not been able to try to find a way to make sure it changes its directory to the one that is the file located at
 *******/

/************
 createUserDirectory
 Creates an user directory that is accessible by that user only
 
 username - takes the name that was specified by the user before
 permission - sets the permission (private,public,etc...)
 
 bugs : 1. needs to specify user directory better.
 
 ************/
void createUserDirectory(char* username, int permission)
{
    if (chdir (PATH) == -1)
    {
        printf ("chdir failed - %s\n", strerror (errno));
    }
    else
    {
        mode_t process_mask = umask(permission);
        int result_code = mkdir(username, S_IRWXU | S_IRWXG | S_IRWXO);
        printf("created directory %s\n ",username);
        umask(process_mask);
    }
}

/***********
 directToUserDirectory
 Directs the user to their user directory
 
 username : takes the name of the specified user
 
 bugs : 1. needs to specify user directory better.
     
******/
void directToUserDirectory(char* username)
{
    char buffer[50];
    int n = 0;
    n = sprintf(buffer, "/users/bryankho/desktop/%s", username); //path need to be change
    //const char * const direction = "/users/bryankho/desktop";
    if (chdir (buffer) == -1)
    {
        printf ("chdir failed - %s\n", strerror (errno));
    }
    
}

/********
 addFileToDirectory
 Inputs an a file into the specified user directory
 
 Method : Creates a file inside the directory and replaces it with the one that is intended
 
 username : takes the name of the specified user
 file: the file which you want to move
 
 bugs : 1. fix file copying
        2. directory need to be specified instead of using a known value
        3. need to specify user directory better
     
 ********/
void addFileToDirectory(char* file, char* username)
{
    
    char buffer[50];
    int n = 0;
   // n = sprintf(buffer, "/users/bryankho/desktop/%s", username);
    if(copy_file(file, filePath2) == 0)
        printf("Copy Successful\n");
    else
        fprintf(stderr, "Error during copy!");
     
    
}

/**********
 deleteFileFromDirectory
 Deletes the file specified by the user
 
 file : takes the location of the file
 
 bugs : 1. need directory fixing like the others
 
 *********/
void deleteFileFromDirectory(char* filename, char* username)
{
    char buffer[50];
    int n = 0;
    n = sprintf(buffer, "/users/bryankho/desktop/%s", username); //path need to be change
    //const char * const direction = "/users/bryankho/desktop";
    if (chdir (buffer) == -1)
    {
        printf ("chdir failed - %s\n", strerror (errno));
    }
    else
    {
        remove(filename);
    }
}
    

/********
     
 *********/
void checkFileExistence(char* filename)
{
    
}
    

/**********
     
 *********/
void fetchListOfFiles(char* name)
{
    
}

/******
 Helper Methods
 ******/

int copy_file(char* source, char* dest)
{
    //int a;
    //int b;
    //char sourceFile[50];
    //char destFile[50];
    //a = sprintf(sourceFile, "%s", source);
    //b = sprintf(destFile, "%s", dest);
    
    size_t len = 0 ;
   // const char a[] = "c:/a/a.exe" ;
   // const char b[] = "d:/b/b.exe" ;
    
    FILE *fp;
    fp=fopen("/users/bryankho/desktop/20714477/file.txt","w");
    
    char buffer[BUFSIZ] = { '\0' } ;
    FILE* in = fopen( source, "rb" ) ;
    FILE* out = fopen( dest, "wb" ) ;
    if( in == NULL || out == NULL )
    {
        perror( "An error occured while opening files!!!" ) ;
        in = out = 0 ;
    }
    else    // add this else clause
    {
        while( (len = fread( buffer, BUFSIZ, 1, in)) > 0 )
        {
            fwrite( buffer, BUFSIZ, 1, out ) ;
        }
        
        fclose(in) ;
        fclose(out) ;
        
        if(remove(source) )
        {
            printf( "File successfully moved. Thank you for using this mini app" ) ;
        }
        else
        {
            printf( "An error occured in deleting the file!!!" ) ;
        }
    }
    return 0 ;
}
 
    
    int main (void)
    {
        //EXECUTE CODE HERE
        //  createUserDirectory("20714477",0);
        //addFileToDirectory(filePath,"20714477");
        deleteFileFromDirectory(filePath3, "20714477");
    }
