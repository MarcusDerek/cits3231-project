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
#include "FileSystem.h"

/************
 createUserDirectory
 Creates an user directory that is accessible by that user only
 
 username - takes the name that was specified by the user before
 permission - sets the permission (private,public,etc...)
 
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

/***********
 
 ******/
void directToUserDirectory(char* username)
{
    
}

/********
 
 ********/
void addFileToDirecoty(char* file, char* username)
{
    
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

int main (void)
{
    //EXECUTE CODE HERE
    createUserDirectory("./testDirectory",0);
}



