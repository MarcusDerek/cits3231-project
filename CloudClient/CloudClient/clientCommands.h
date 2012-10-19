//
//  clientCommands.h
//  CloudClient
//
//  Created by Lion User on 14/10/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef COMMANDS_H
#define COMMANDS_H

char* get_IntroMsg();
char* get_HelpList();
int verifyUserCommand();
char* registerNewAccount();
char* loginToAccount();
char* addFile();
char* concatSentence(int command, char* sentence1, char* sentence2);




#endif
