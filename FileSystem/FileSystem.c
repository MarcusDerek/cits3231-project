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
 
 username : takes the name of the specified user
 file: the file which you want to move
 
 bugs : NOT YET IMPLEMENTED
     
 ********/
void addFileToDirectory(char* file, char* username)
{
    /*
    char buffer[50];
    int n = 0;
    n = sprintf(buffer, "/users/bryankho/desktop/%s", username);
    if(copy_file(file, buffer) == 0)
        printf("Copy Successful\n");
    else
        fprintf(stderr, "Error during copy!");
     */
    
}

    /**********
     
     *********/
    void deleteFileFromDirectory(char* file)
    {
        
    }
    
    
    /********
     
     *********/
    void checkFileExistance(char* filename)
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
/*
int copy_file(char *old_filename, char  *new_filename)
{
    FILE  *ptr_old, *ptr_new;
    errno_t err = 0, err1 = 0;
    int  a;
    
    err = fopen_s(&ptr_old, old_filename, "rb");
    err1 = fopen_s(&ptr_new, new_filename, "wb");
    
    if(err != 0)
        return  -1;
    
    if(err1 != 0)
    {
        fclose(ptr_old);
        return  -1;
    }
    
    while(1)
    {
        a  =  fgetc(ptr_old);
        
        if(!feof(ptr_old))
            fputc(a, ptr_new);
        else
            break;
    }
    
    fclose(ptr_new);
    fclose(ptr_old);
    return  0;
}
 */
    
    int main (void)
    {
        //EXECUTE CODE HERE
       //  createUserDirectory("20714477",0);
    }
