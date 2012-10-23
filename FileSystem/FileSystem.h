//
//  FileSystem.h
//  CloudClient
//
//  Created by Bryan Kho on 14/10/2012.
//  Copyright (c) 2012 . All rights reserved.
//

#ifndef COMMANDS_H
#define COMMANDS_H


//FileSystem Variables
 const char * const PATH = "/users/bryankho/desktop"; //path needs to be changed to accomodate 


//FileSystem Call Functions
void createUserDirectory(char* username,int permission); //to be change to more simpler terms
void directToUserDirectory(char* username);
void addFileToDirectory(char* file,char* username);
void deleteFileFromDirectory(char* filename);
void checkFileExistance(char* filename);
void fetchListOfFiles(char* username);
int copy_file(char *old_filename, char  *new_filename);


//BankServer Call Functions
void checkUserBalance(char* username);


#endif
