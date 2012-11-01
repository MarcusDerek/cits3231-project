/*  commands.h
 * 
 * Team: 
 * Marcus Derek - 11016403
 * Taiga Yano - 20698782
 * Bryan Kho - 20714477
 */

#ifndef COMMANDS_H
#define COMMANDS_H

/*Global Var*/
char* get_IntroMsg();
char* get_HelpList();
int verifyUserCommand();
char* registerNewAccount();
char* loginToAccount();
char* addFile();
char* deleteFile();
char* fetchFile();
char* verifyFile();
char* listAllFiles();
char* buyCloudMoney(); char* getAmountToBuy();
char* checkBankFunds();
char* checkCloudFunds();
char* registerBankAccount();
char* concatSentence(int command, char* sentence1, char* sentence2);




#endif
