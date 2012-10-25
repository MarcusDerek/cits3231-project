//
//  FileSystem.h
//  FileSystem
//
//  Created by Bryan Kho on 14/10/2012.
//  Copyright (c) 2012 . All rights reserved.
//

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

//FileSystem Variables

//FileSystem Call Functions
void createUserDirectory(char* username);
void addFileToDirectory(char* file,char* username);
int deleteFileFromDirectory(char* filename, char* username);
int checkFileExistence(char* filename, char* username);
char* fetchListOfFiles(char* username);
int addToPasswordFile(char* username, char* password);

//helper methods from filesystem
int verifyIfPasswordExist(char* username);
char* extractFileName(char *filePath);

//BankServer Call Functions
void checkUserBalance(char* username);

#endif
