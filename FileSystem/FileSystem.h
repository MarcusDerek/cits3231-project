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
const char * const PATH = "/users/bryankho/desktop"; //path needs to be changed to accomodate
char* filePath = "/users/bryankho/desktop/testFile.rtf"; //testerFile pathway to test addFileToDirectory
char* filePath2 = "/users/bryankho/desktop/movablefile.txt"; //testerFile pathway to test addFileToDirectory
char* filePath3 = "/users/bryankho/desktop/MarcusDerek/file.txt"; //deletion testing
char* directoryCopy; //for addFileToDirectory function



//FileSystem Call Functions
void createUserDirectory(char* username,int permission); //to be change to more simpler terms
void directToUserDirectory(char* username);
void addFileToDirectory(char* file,char* username);
void deleteFileFromDirectory(char* filename, char* username);
int checkFileExistence(char* filename, char* username);
void fetchListOfFiles(char* username);
int addToPasswordFile(char* username, char* password);

//helper methods from filesystem
int copy_file(char *old_filename, char *new_filename);
int copyFiles(char* source_file, char* target_file);
int verifyIfPasswordExist(char* username);

//BankServer Call Functions
void checkUserBalance(char* username);


#endif
