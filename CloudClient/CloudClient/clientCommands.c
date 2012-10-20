//
//  clientCommands.c
//  CloudClient
//
//  Created by Lion User on 14/10/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char list[][30] = {
    "-exit", //0
    "-registerNewAccount", //1
    "-login", //2
    "-help", //3
    "-addFile", //4
    "-deleteFile", //5
    "-fetchFile",//6
    "-verifyFile",//7
            
};

//---------------
// Messages
//---------------          
/**
 * Intro message for Client
 * @return introduction message
 */
char* get_IntroMsg() {
    char *message = "Welcome to Internix Cloud Service Client!\n\n"
    "If you are a NEW USER, please register an account with us using:\n"
    "-register user-name password\n\n"
    "If you are a returning customer, please proceed to log in to your account using:"
    "-login user-name password\n\n"
    "If you need assistance at any point in time,\n"
    "type -help to access list of commands available.\n\n"
    "Thank you for choosing us as your cloud storage provider!\n"
    "- Devs of Internix Cloud Services\n\n\n"
    "Please wait while we connect you to our server...\n\n";
    return message;
}

char* get_HelpList() {
    char *listOfCommands = "List of Commands\n\n"
    "-registerNewAccount user-name password || To register for a new account\n"
    "-login user-name password || To log in to your existing account\n"
    "-help || Returns a list of useful commands\n";
    return listOfCommands;
}
/**
 * For concatenating 2 strings
 * @param sentence1 
 * @param sentence2
 * @return concatenated string
 */
char* concatSentence(int command, char* sentence1, char* sentence2) {
    char *output = malloc(1000 * sizeof(char));
    sprintf(output, "%d ", command);
    strcat(output, sentence1);
    strcat(output, " ");
    strcat(output, sentence2);
    return output;
}
/**
 * Verify the user inputs in the client by checking against a pre-determined list
 * @return an integer that correspond to the command received.
 */
int verifyUserCommand(char* cmd) {
    
    int ERROR = 404;
    
    if(strcmp(cmd, list[0]) == 0) { //-exit
        return 0;        
    } else if(strcmp(cmd, list[1]) == 0) { //-registerNewAccount USERNAME PASSWORD
        return 1;
    } else if(strcmp(cmd, list[2]) == 0) { //-login USERNAME PASSWORD
        return 2;
    } else if(strcmp(cmd, list[3]) == 0) { //-help
        return 99;
    } else if(strcmp(cmd, list[4]) == 0) { //-addFile
        return 3;
    } else if(strcmp(cmd, list[5]) == 0) { //-deleteFile
        return 4;
    } else if(strcmp(cmd, list[6]) == 0) { //-fetchFile
        return 5;
    } else if(strcmp(cmd, list[7]) == 0) { //-verifyFile
        return 6;
    }
    return ERROR;
}
//----------------------
//User command Section
//----------------------
/**
 * Registers a new account
 * @return Combination of output ie. (1 USERNAME PASSWORD)
 */
char* registerNewAccount() { //1
    char *userName = malloc(1000 * (sizeof(char)));
    char *password = malloc(1000 * (sizeof(char)));
    
    char *output = malloc(1000 * sizeof(char));
    printf("Registering account...\n");
    printf("Enter your desired username: ");
    scanf("%s", userName);
    printf("Enter a 6-digit password: ");
    scanf("%s", password);
    output = concatSentence(1,userName, password); //1 = registerAcc
    printf("Userid: %s Password: %s | Output from Main: %s\n",userName, password, output);
    return output;
}

char * loginToAccount() {//2
    char *userName = malloc(1000 * (sizeof(char)));
    char *password = malloc(1000 * (sizeof(char)));
    
    char *output = malloc(1000 * sizeof(char));
    printf("Logging into account...\n");
    printf("Enter your username: ");
    scanf("%s", userName);
    printf("Enter your 6-digit password: ");
    scanf("%s", password);
    output = concatSentence(2,userName, password); //2 = login
    printf("Userid: %s Password: %s | Output from Main: %s\n",userName, password, output);
    return output;
}
char* addFile() {
    char *fullPathName = malloc(1000 * (sizeof(char)));
    char *password = malloc(1000 * (sizeof(char)));
    
    char *output = malloc(1000 * sizeof(char));
    printf("-- Add File Command initiated --\n");
    printf("Please specify the FULL path of the file you wish to upload below.\n File Path: ");
    scanf("%s", fullPathName);
    strcpy(output, fullPathName);
    printf("File: %s ----- SELECTED\n", output);
    return output;
}
