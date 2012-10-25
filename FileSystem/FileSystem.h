//
//  FileSystem.h
//  FileSystem
//
//  Created by Bryan Kho on 14/10/2012.
//  Copyright (c) 2012 . All rights reserved.
//

#ifndef COMMANDS_H
#define COMMANDS_H

//FileSystem Variables
char* filePath2 = "/users/bryankho/desktop/movefile.rtf"; //tester file that needs to be moved
char* directoryCopy;

//FileSystem Call Functions
void createUserDirectory(char* username);
void addFileToDirectory(char* file,char* username);
int deleteFileFromDirectory(char* filename, char* username);
int checkFileExistence(char* filename, char* username);
char* fetchListOfFiles(char* username);
int addToPasswordFile(char* username, char* password);

//helper methods from filesystem
int copy_file(char *old_filename, char *new_filename);
int copyFiles(char* source_file, char* target_file);
int verifyIfPasswordExist(char* username);
char* extractFileName(char *filePath);

//BankServer Call Functions
void checkUserBalance(char* username);

#endif
