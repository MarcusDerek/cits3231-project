//
//  FileSystem.c
//  FileSystem
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

#include "FileSystem.h"

/**
 createUserDirectory
 Creates a folder for the specified user
 
 @username Takes the name that is specified by the user
 @permission sets the permission
 */

void createUserDirectory(char* username)
{
    int permission = 999;
    char* pathing = malloc(1000 * sizeof(char));
    strcat(pathing, "Storage/");
    strcat(pathing, username);
    mkdir(pathing,permission);
}

/**
 addFileToDirectory
 Moves the file into the directory (copying)
 
 @file Takes the name of the file
 @username Takes the name that is specified by the user
 
 */

void addFileToDirectory(char* file, char* username)
{
    char* creation = malloc(1000 * sizeof(char));
    char* destination = malloc(1000 * sizeof(char));
    char* source = malloc(1000 * sizeof(char));
    
    source = extractFileName(file);
    
    strcat(destination,"Storage/");
    strcat(destination,username); /* destination of the file that needs to be transferred */
    
    FILE * create;

    creation = destination; /* the files that needs to be replaced */
    strcat(creation,"/");
    strcat(creation,source);
    
    create = fopen(creation,"W");
    
    if ( rename(file,creation) )
    {
      //  puts ( "File successfully renamed" );
    }
    else
      //  perror( "Error renaming file" );
    
    fclose(create);
        
}

/**
 deleteFileFromDirectory
 Delete the file in the directory

 @filename Takes the name of the file
 @username Takes the name that is specified by the user
 
 */

int deleteFileFromDirectory(char* filename, char* username)
{
    char* filePathing = malloc(1000 * sizeof(char));
    strcat(filePathing,"Storage/");
    strcat(filePathing,username);
    strcat(filePathing,"/");
    strcat(filePathing,filename);
    
    if(remove(filePathing))
        return 1;
    else
        return 0;
}

/**
 checkFileExistence
 Check the existence of the file in the diretory
 
 @filename Takes the name of the file
 @username Takes the name that is specified by the user
 
 */

int checkFileExistence(char* filename, char* username)
{
    char* filePathing = malloc(1000 * sizeof(char));
    strcat(filePathing,"Storage/");
    strcat(filePathing,username);
    strcat(filePathing,"/");
    strcat(filePathing,filename);
    
    FILE *f = fopen(filePathing, "r");
    
    if (!f)
        return 0;
    
    fclose(f);
    
    return 1;
    
}

/**
 fetchListOfFiles
 Prints all the file names into a string
 
 @username Takes the name that is specified by the user
 
 */

char* fetchListOfFiles(char* username)
{
    char* filePathing = malloc(1000 * sizeof(char));
    strcat(filePathing,"Storage/");
    strcat(filePathing,username);
    
    DIR *dirp;
    dirp = opendir(filePathing);
  
    char* data = malloc(1000 * sizeof(char));
    struct dirent *dp;

    while ((dp = readdir(dirp)) != NULL)
    {
        if (strcmp(dp->d_name,".") && strcmp(dp->d_name,".."))
        {
            struct stat statbuf;
            stat(dp->d_name, &statbuf);
            strcat(data,dp->d_name);
            strcat(data,"\n");
        }
        else
        {
            dp++;
        }
          
    }
    printf("%s\n",data);
    return data;
}

/**
 addToPasswordFile
 Add new Password and Username into passwordlist.txt
 
 @username Takes the name that is specified by the user
 @password Takes the password specified
 
 */

int addToPasswordFile(char* username, char* password)
{
    int checker = verifyIfPasswordExist(username);
    
    if( checker == 1 )
    {
        return 0;
    }
    
    char* filePathing = malloc(1000 * sizeof(char));
    char* data = malloc(1000* sizeof(char));
    
    strcat(data, username);
    strcat(data, " ");
    strcat(data, password);
    strcat(data,"\n");
    strcat(filePathing,"Password/passwordlist.txt");
    
    FILE *fp = fopen(filePathing, "a+");
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
    size_t len = 0 ;

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
    
    FILE *source, *target;
    
    source = fopen(source_file, "r");
    
    if( source == NULL )
    {
        printf("Press any key to exit...\n");
        exit(EXIT_FAILURE);
    }
        
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
			find_result++;
            fclose(fp);
            return 1;
		}
		line_num++;
	}

    
     fclose(fp);
    return 0;
    
}


char* extractFileName(char* filePath)
{
    char* copyFilePath = malloc(1000 * sizeof(char));
    strcpy(copyFilePath,filePath);
    char* pch;
    char* fileName;
    
    pch = strchr(copyFilePath,'/');
    while (pch!=NULL)
    {
        pch = strchr((pch+1),'/');
        
        if (pch != NULL)
        {
            fileName = pch;
            fileName++;
        }
           
    }
    return fileName;
}

    
int main (void) /* Test Code Here */
{
  
        // createUserDirectory("BryanKho");
       //  addFileToDirectory(filePath2,"BryanKho");
      //   deleteFileFromDirectory("dota2.exe", "BryanKho");
      //  int test1 = checkFileExistence("dota2.exe","BryanKho");
      //  printf("%i\n",test1);
      //   fetchListOfFiles("BryanKho");
        // addToPasswordFile("Bob", "IamBob");
        // addToPasswordFile("Jane", "ComplicatedPassword");
        // addToPasswordFile("Marcus", "123456");
        // addToPasswordFile("Amin","IamIndian");
        // addToPasswordFile("BryanKho","donttelllanyone");
        // verifyIfPasswordExist("Marcus");
}
