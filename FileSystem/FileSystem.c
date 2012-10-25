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
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>
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
 
 bugs :
 
 ************/
void createUserDirectory(char* username, int permission)
{
    /*if (chdir (PATH) == -1)
    {
        printf ("chdir failed - %s\n", strerror (errno));
    }
    else
    { 
    char pathing[80];
    strcat(pathing, "Storage/");
    strcat(pathing, username);
        mode_t process_mask = umask(permission);
        int result_code = mkdir(pathing, S_IRWXU | S_IRWXG | S_IRWXO);
        printf("created directory %s\n ",username);
        umask(process_mask);
    }
     */
    
    char* pathing = malloc(1000 * sizeof(char));
    //sprintf(fullPathName,"%s/%s", FILESERVER_AREA, userName);
    strcat(pathing, "Storage/");
    strcat(pathing, username);
    mkdir(pathing,permission);
   // printf("Created Directory at: %s\n", fullPathName);
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
    n = sprintf(buffer, "Storage/%s", username); //path need to be change
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
  //  char memory[200];
  //  memory = file;
    char* base = malloc(1000 * sizeof(char));
  //  printf("here2\n");
     *base =  basename(file);
   // printf(base);
   // printf("\n");
   // printf("here5\n");
    char* buffer = malloc(1000 * sizeof(char));
    //int n = 0;
    
   // printf("here4\n");
    FILE *fileCopied;
 
   // printf("here3\n");
    strcat(buffer,"Storage/");
    strcat(buffer,username);
    strcat(buffer,"/");
   // printf("this is the error 2\n");
    strcat(buffer,base);
   // printf("this is the error\n");
    fileCopied = fopen(buffer,"W+"); // TO CREATE THE NEW FILE
   // n = sprintf(buffer, "/users/bryankho/desktop/%s", username);
    
    //NEED TO CREATE AN EXACT SAME FILE AT THE USER DIRECTORY
    //printf("here1\n");
    if(copyFiles(file, buffer) == 0)
        printf("Copy Successful\n");
    else
        fprintf(stderr, "Error during copy!");
    
}

/**********
 deleteFileFromDirectory
 Deletes the file specified by the user
 
 file : takes the location of the file
 
 bugs :
 *********/
void deleteFileFromDirectory(char* filename, char* username)
{
    char filePathing[100];
    strcat(filePathing,"Storage/");
    strcat(filePathing,username);
    strcat(filePathing,"/");
    strcat(filePathing,filename);
    remove(filePathing);
  
    /*
    char buffer[50];
    int n = 0;
    n = sprintf(buffer, "Storage/%s", username); //path need to be change
    //const char * const direction = "/users/bryankho/desktop";
    if (chdir (buffer) == -1)
    {
        printf ("chdir failed - %s\n", strerror (errno));
    }
    else
    {
        remove(filePathing);
    }
     */
}
    

/********
 1. to do find files
     
 *********/
void checkFileExistence(char* filename, char* username)
{
    char filePathing[100];
    strcat(filePathing,"Storage/");
    strcat(filePathing,username);
    strcat(filePathing,"/");
    strcat(filePathing,filename);
    
    
    struct stat st;
    //if(stat("/tmp",&st) == 0)
      //  printf(" /tmp is present\n");
    if (stat(filePathing,&st) == -1)
    {
        printf("could not find file\n");
        //return false;
    } else {
        printf("FOUND IT\n");
        //return true;
    }
}


/**********
     
 1. skip first two pointers
 *********/
void fetchListOfFiles(char* username)
{
   // char* base = malloc(1000 * sizeof(char));
    char* filePathing = malloc(1000 * sizeof(char));
    strcat(filePathing,"Storage/");
    strcat(filePathing,username);
    
    DIR *dirp;
    dirp = opendir(filePathing);
    //dirp = opendir(PATH);
  
    
    char* data = malloc(1000 * sizeof(char));
    struct dirent *dp;
   //   printf("here\n");
    while ((dp = readdir(dirp)) != NULL)
    {
                              // printf("here\n");
        struct stat statbuf;
        stat(dp->d_name, &statbuf);
        printf("%s\n",dp->d_name);
       // data = malloc(1000 * sizeof(char));
       // *data = malloc(1000 * sizeof(char));
        strcat(data,dp->d_name);
        strcat(data,"\n");
       // data++;
        
    }
    
    strcat(filePathing,"/list.txt");
    remove(filePathing);
    FILE *fp = fopen(filePathing, "ab");
    if (fp != NULL)
    {
        fputs(data, fp);
        fclose(fp);
    }
}

/*****
 
 *****/

int addToPasswordFile(char* username, char* password)
{
    int checker = verifyIfPasswordExist(username);
    
    if( checker == 1 )
    {
        printf("fail, since an user with the same name is already in here\n");
        return 0;
    }
    
    char* filePathing = malloc(1000 * sizeof(char));
    char* data = malloc(1000* sizeof(char));
  //  strcat(filePathing,"Storage/");
   // strcat(filePathing,username);
   // strcat(filePathing,"/");
    strcat(data, username);
    strcat(data, " ");
    strcat(data, password);
    strcat(data,"\n");
    strcat(filePathing,"Password/passwordlist.txt");
    FILE *fp = fopen(filePathing, "ab");
    if (fp != NULL)
    {
        fputs(data, fp);
        fclose(fp);
    }
    return 1;
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
    
    
    source;
    FILE *fp;
    fp=fopen(source,"w");
    
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

int copyFiles(char* source_file, char* target_file)
{
    char ch;
   // char ch, source_file[20], target_file[20];
    FILE *source, *target;
    
 //   printf("Enter name of file to copy\n");
 //   gets(source_file);
    
    source = fopen(source_file, "r");
    
    if( source == NULL )
    {
        printf("Press any key to exit...\n");
        exit(EXIT_FAILURE);
    }
    
 //   printf("Enter name of target file\n");
 //   gets(target_file);
    
    target = fopen(target_file, "w");
    
    if( target == NULL )
    {
        fclose(source);
        printf("Press any key to exit...\n");
        exit(EXIT_FAILURE);
    }
    
    while( ( ch = fgetc(source) ) != EOF )
        fputc(ch, target);
    
    printf("File copied successfully.\n");
    
    fclose(source);
    fclose(target);
    
    return 0;
}

int verifyIfPasswordExist(char* username)
{
    char* filePathing = malloc(1000 * sizeof(char));
    char* data = malloc(1000* sizeof(char));
    char* compare = malloc(1000* sizeof(char));
    int find_result = 0;
    int line_num = 1;
    char* temp = malloc(1000 * sizeof(char));
    
    strcat(data, username);
    strcat(filePathing,"Password/passwordlist.txt");
    
    FILE *fp;
    
    if((fp = fopen(filePathing, "r")) == NULL)
    {
        	return(-1);
    }
    
    while(fgets(temp, 512, fp) != NULL) {
		if((strstr(temp, username)) != NULL) {
			//printf("A match found on line: %d\n", line_num);
			//printf("\n%s\n", temp);
			find_result++;
            fclose(fp);
            return 1;
		}
		line_num++;
	}

    /*
    if (fp != NULL)
    {
        fscanf(fp, "%s", compare);
        if (compare == data)
        {
            printf("%s\n",compare);
            printf("found it\n");
            //do something
        } else {
            printf("%s\n",compare);
            printf("cant find\n");
        }
    }
     */
    
     fclose(fp);
    return 0;
    
}
 
    
    int main (void)
    {
        //EXECUTE CODE HERE
        //createUserDirectory("MarcusDerek",111);
      //  addFileToDirectory(filePath2,"MarcusDerek");
        //deleteFileFromDirectory("file.txt", "MarcusDerek");
       // checkFileExistence("marcus.rtf","MarcusDerek");
       // fetchListOfFiles("MarcusDerek");
        addToPasswordFile("Bob", "IamBob");
      //  verifyIfPasswordExist("Marcus");
    }
