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


//FileSystem Call Functions
void createUserDirectory(char* username,char* permission);
void direcToUserDirectory(char* username);
void addFileToDirectory(char* file,char* username);
void deleteFileFromDirectory(char* filename);
void checkFileExistance(char* filename);
void fetchListOfFiles(char* username);

//BankServer Call Functions
void checkUserBalance(char* username);


#endif
