#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>

/*
Programmers: Steeve Soni & Shanda Wu
Date: December 11th, 2019
Purpose: This is the fourth assignment called DUMB protocol where we are using a server
and client and sending messages between them and the user. The main reason for this project
is to learn how the server and client can communicate as well as working with pthreads
mutex locks and unlocks


*/


typedef struct charByChar{
	char  oneChar;
	struct charByChar * nextChar;
}charByChar;

// This is a linked list that contains an array to store each message in the current message box.
// When a new message wants to be stored inside of a certain message box you should make a new message
typedef struct Messages{
	char * theMessage;
	struct Messages * nextMessage;
}Messages;

//This is a linked list of a linked list of all the messages in each box. When you want to enter a certain message box, you have all the boxes in a list, you find which box you want to enter
// then you store it after the last message of that message box.
typedef struct MessageBoxes{
	struct MessageBoxes * nextBox;
	struct Messages* goToMessages;
	char* theBoxName;
	pthread_mutex_t boxLocks;
	
}MessageBoxes;

typedef struct Descriptors{
	int descriptorNum;
	char * ipAddress;
}Descriptors;

MessageBoxes *firstBox;

pthread_mutex_t createLock = PTHREAD_MUTEX_INITIALIZER;


void* talkToClient (void* args){

	time_t t = time(NULL);
	time(&t);
	struct tm *tm;
	tm = localtime(&t);
	char currentTime[256];
	
	Descriptors * thisDescriptor = (Descriptors*) args;
	
	if(thisDescriptor->descriptorNum < 0){
		
		pthread_exit(0);
		
	}
	else{
		time(&t);
		strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
		fprintf(stdout, "%s %s connected\n", currentTime, thisDescriptor->ipAddress);
	}
	char helloCommand[6];
	
	char commands[7];
	//printf("size of hello command is: %d\n", strlen(helloCommand));
	read(thisDescriptor->descriptorNum, helloCommand, 5);
	//helloCommand[6] = '\0';

	if(strcmp(helloCommand, "HELLO") == 0){
		char * sayHello = "HELLO DUMBv0 ready!";
		write(thisDescriptor->descriptorNum, sayHello, strlen(sayHello));
		time(&t);
		tm = localtime(&t);
		strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
		fprintf(stdout, "%s %s HELLO\n", currentTime, thisDescriptor->ipAddress);
		
	}
	else{
		char * notHello = "You were supposed to say HELLO but didn't! Goodbye!";
		write(thisDescriptor->descriptorNum, notHello, strlen(notHello));
		time(&t);
		tm = localtime(&t);
		strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
		fprintf(stderr, "%s %s ER:NOHELLO\n", currentTime, thisDescriptor->ipAddress);
		pthread_exit(0);
	}
	bzero(helloCommand, strlen(helloCommand));
	while(1){
		bzero(commands,strlen(commands));

		read(thisDescriptor->descriptorNum,commands, 6);

		// If the client sends you GDBYE that means they want to end the connection
		if(strncmp(commands, "GDBYE", 5) == 0){
			write(thisDescriptor->descriptorNum,"\0", 1);
			time(&t);
			tm = localtime(&t);
			strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
			fprintf(stdout, "%s %s disconnected\n", currentTime, thisDescriptor->ipAddress);
			break;
		}
		// If the client sends you CREAT that means they want to create a new message box so you should check the proper parameters to make sure they entered
		// the right length as well as to make sure what they entered was not already a message box name.
		else if(strcmp(commands, "CREAT ") == 0){
			
			char length[1];
			char *boxName = (char*) malloc(sizeof(char)*26);
			int i = 0;
			//bzero(boxName, strlen(boxName));
			read(thisDescriptor->descriptorNum, boxName, 26);
			while(i < 26){

				if(boxName[i] == '\0'){
					break;
				}
				i++;
			}


			if(i == 25){
				read(thisDescriptor->descriptorNum, length, 1);
			}

			// length is less than 5 should be an error
			if(i < 5){
				time(&t);
				tm = localtime(&t);
				strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
				fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
				write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
				
			}
			// length equals 26 means this is an error since it is longer than 25 characters
			else if(i == 26){
				char readRest[1];
				read(thisDescriptor->descriptorNum, readRest, 1);
				while(readRest[0] != '\0'){
					bzero(readRest, 1);
					read(thisDescriptor->descriptorNum, readRest, 1);
				}
				read(thisDescriptor->descriptorNum, readRest, 1);
				time(&t);
				tm = localtime(&t);
				strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
				fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
				write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
			}
			// the first character is a number which should never be the case. it should always start with an alphabet
			else if(isalpha(boxName[0]) == 0){
				time(&t);
				tm = localtime(&t);
				strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
				fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
				write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
			}
			else{
				
				pthread_mutex_lock(&createLock);
				if(firstBox == NULL){
					//you should create the first message box
					MessageBoxes* holderBox = (MessageBoxes*) malloc(sizeof(MessageBoxes));
					holderBox->theBoxName = (char*) malloc(sizeof(char) * 26);
					holderBox->theBoxName = boxName;
					holderBox->nextBox = NULL;
					holderBox->goToMessages = NULL;
					holderBox->boxLocks = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
					firstBox = holderBox;
					time(&t);
					tm = localtime(&t);
					strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
					fprintf(stdout, "%s %s CREAT\n", currentTime, thisDescriptor->ipAddress); // the first box was successfully created
					write(thisDescriptor->descriptorNum, "OK!", 3);
				}
				else{

					int alreadyExists = 0;
					MessageBoxes* currentBox;
					currentBox = firstBox;

					while(currentBox != NULL){
						//printf("currentbox name is\n: %s", currentBox->theBoxName);
						//printf("we are comparing: %s and %s\n", currentBox->nextBox->theBoxName, boxName);
						if(strcmp(currentBox->theBoxName, boxName) == 0){
							alreadyExists = 1;
							break;
						}
						if(currentBox->nextBox == NULL){
							break;
						}
						currentBox = currentBox->nextBox;
					}
					// you are trying to enter in a box that already exists. should be an error
					if(alreadyExists == 1){
						alreadyExists = 0;
						time(&t);
						tm = localtime(&t);
						strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
						fprintf(stderr,"%s %s ER:EXIST\n", currentTime, thisDescriptor->ipAddress);	
						write(thisDescriptor->descriptorNum, "ER:EXIST", 8);
						
					}
					else{
						//the box does not exist and it can be successfully created!
						currentBox->nextBox = (MessageBoxes*) malloc(sizeof(MessageBoxes));
						currentBox = currentBox->nextBox;
						currentBox->theBoxName = (char*) malloc(sizeof(char) * 26);
						currentBox->theBoxName = boxName;
						currentBox->goToMessages = NULL;
						currentBox->nextBox = NULL;
						currentBox->boxLocks = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
						time(&t);
						tm = localtime(&t);
						strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
						fprintf(stdout, "%s %s CREAT\n", currentTime, thisDescriptor->ipAddress);
						write(thisDescriptor->descriptorNum, "OK!", 3);
					}
					
					
				}
				pthread_mutex_unlock(&createLock);
			
			}

		}
		else if(strcmp(commands,"DELBX ") == 0){

			char length[1];
			char *boxName = (char*) malloc(sizeof(char)*26);
			int i = 0;
			//bzero(boxName, strlen(boxName));
			read(thisDescriptor->descriptorNum, boxName, 26);
		
			while(i < 26){

				if(boxName[i] == '\0'){
					break;
				}
				i++;
			}


			if(i == 25){
				read(thisDescriptor->descriptorNum, length, 1);
			}

			// Less than 5 characters means it is malformed and that is an error what!
			if(i < 5){
				time(&t);
				tm = localtime(&t);
				strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
				fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
				write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
				
			}
			// greater than 25 characters means error what! this is too long.
			else if(i == 26){
				char readRest[1];
				read(thisDescriptor->descriptorNum, readRest, 1);
				while(readRest[0] != '\0'){
					bzero(readRest, 1);
					read(thisDescriptor->descriptorNum, readRest, 1);
				}
				read(thisDescriptor->descriptorNum, readRest, 1);
				time(&t);
				tm = localtime(&t);
				strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
				fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
				write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
			}
			//first character should not be a number another error what!
			else if(isalpha(boxName[0]) == 0){
				time(&t);
				tm = localtime(&t);
				strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
				fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
				write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
			}
			else{
				int exists = 0;
				int count = 0;
				MessageBoxes* currentBox;
				currentBox = firstBox;
				MessageBoxes* prev;
				while(currentBox != NULL){
						
					if(strcmp(currentBox->theBoxName, boxName) == 0){
						count ++;
						exists = 1;
						break;
					}
					
					if(currentBox->nextBox == NULL){
						break;
					}
					prev = currentBox;
					currentBox = currentBox->nextBox;
					
				}
				if(exists == 1 && count == 1 && currentBox->nextBox == NULL){
					// error the box is opened by someone else
					if(pthread_mutex_trylock(&currentBox->boxLocks) != 0){
							time(&t);
							tm = localtime(&t);
							strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
							fprintf(stderr,"%s %s ER:OPEND\n", currentTime, thisDescriptor->ipAddress);	
							write(thisDescriptor->descriptorNum, "ER:OPEND", 8);
						
					}
					else{
						// error you can't delete because there are messages in that box still
						if(currentBox->goToMessages != NULL){
							time(&t);
							tm = localtime(&t);
							strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
							fprintf(stderr,"%s %s ER:NOTMT\n", currentTime, thisDescriptor->ipAddress);	
							write(thisDescriptor->descriptorNum, "ER:NOTMT", 8);
							pthread_mutex_unlock(&currentBox->boxLocks);
							
							
						}
						// if it is empty we can delete this one box
						else if(currentBox->goToMessages == NULL){
							firstBox = NULL;
							exists = 0;
							time(&t);
							tm = localtime(&t);
							strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
							fprintf(stdout,"%s %s DELBX\n", currentTime, thisDescriptor->ipAddress);	
							write(thisDescriptor->descriptorNum, "OK!", 3);
							pthread_mutex_unlock(&currentBox->boxLocks);
							
							
						}
						
					}
						
				}else if(exists == 1 && count == 1 && currentBox->nextBox != NULL){
					if(pthread_mutex_trylock(&currentBox->boxLocks) != 0){
						time(&t);
						tm = localtime(&t);
						strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
						fprintf(stderr,"%s %s ER:OPEND\n", currentTime, thisDescriptor->ipAddress);	
						write(thisDescriptor->descriptorNum, "ER:OPEND", 8);
					}
					else{
						// messages are not empty
						if(currentBox->goToMessages != NULL){
							time(&t);
							tm = localtime(&t);
							strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
							fprintf(stderr,"%s %s ER:NOTMT\n", currentTime, thisDescriptor->ipAddress);	
							write(thisDescriptor->descriptorNum, "ER:NOTMT", 8);
							pthread_mutex_unlock(&currentBox->boxLocks);
							
						}
						else if(currentBox->goToMessages == NULL){
							// you can successfully delete the box
							firstBox = currentBox->nextBox;
							exists = 0;
							time(&t);
							tm = localtime(&t);
							strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
							fprintf(stdout,"%s %s DELBX\n", currentTime, thisDescriptor->ipAddress);	
							write(thisDescriptor->descriptorNum, "OK!", 3);
							pthread_mutex_unlock(&currentBox->boxLocks);
							
						}
						
					}
						
				}else if(exists == 1 && count != 1){
					// someone already has the box open
					if(pthread_mutex_trylock(&currentBox->boxLocks) != 0){
						time(&t);
						tm = localtime(&t);
						strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
						fprintf(stderr,"%s %s ER:OPEND\n", currentTime, thisDescriptor->ipAddress);	
						write(thisDescriptor->descriptorNum, "ER:OPEND", 8);
					}
					else{
						//as long as the messages are empty we can successfully delete this box
						if(currentBox->nextBox == NULL && currentBox->goToMessages == NULL){
							prev->nextBox = NULL;
							time(&t);
							tm = localtime(&t);
							strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
							fprintf(stdout,"%s %s DELBX\n", currentTime, thisDescriptor->ipAddress);	
							write(thisDescriptor->descriptorNum, "OK!", 3);
							pthread_mutex_unlock(&currentBox->boxLocks);
							
						}else if (currentBox->nextBox != NULL && currentBox->goToMessages == NULL){
							prev->nextBox = currentBox->nextBox;
							time(&t);
							tm = localtime(&t);
							strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
							fprintf(stdout,"%s %s DELBX\n", currentTime, thisDescriptor->ipAddress);	
							write(thisDescriptor->descriptorNum, "OK!", 3);
							pthread_mutex_unlock(&currentBox->boxLocks);
							
						}
						else if(currentBox->nextBox == NULL && currentBox->goToMessages != NULL){
							time(&t);
							tm = localtime(&t);
							strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
							fprintf(stderr,"%s %s ER:NOTMT\n", currentTime, thisDescriptor->ipAddress);	
							write(thisDescriptor->descriptorNum, "ER:NOTMT", 8);
							pthread_mutex_unlock(&currentBox->boxLocks);
							
						}
						else if(currentBox->nextBox != NULL && currentBox->goToMessages != NULL){
							time(&t);
							tm = localtime(&t);
							strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
							fprintf(stderr,"%s %s ER:NOTMT\n", currentTime, thisDescriptor->ipAddress);	
							write(thisDescriptor->descriptorNum, "ER:NOTMT", 8);
							pthread_mutex_unlock(&currentBox->boxLocks);
							
						}
						
					}
						
				}
				else{
					time(&t);
					tm = localtime(&t);
					strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
					fprintf(stderr, "%s %s ER:NEXST\n", currentTime, thisDescriptor->ipAddress);
					write(thisDescriptor->descriptorNum, "ER:NEXST", 8);
				}
					
			}


			
		}
		
		else if(strcmp(commands, "OPNBX ") == 0){
			char length[1];
			char *boxName = (char*) malloc(sizeof(char)*26);
			int i = 0;
			read(thisDescriptor->descriptorNum, boxName, 26);

			while(i < 26){

				if(boxName[i] == '\0'){
					break;
				}
				i++;
			}
			if(i == 25){
				read(thisDescriptor->descriptorNum, length, 1);
			}
			if(i < 5){
				// it is not long enough so it should be error what
				time(&t);
				tm = localtime(&t);
				strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
				fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
				write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
				
			}			
			else if(i == 26){
				// it is too long so it should be error what
				char readRest[1];
				read(thisDescriptor->descriptorNum, readRest, 1);
				while(readRest[0] != '\0'){
					bzero(readRest, 1);
					read(thisDescriptor->descriptorNum, readRest, 1);
				}
				read(thisDescriptor->descriptorNum, readRest, 1);
				time(&t);
				tm = localtime(&t);
				strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
				fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
				write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
			}
			else if(isalpha(boxName[0]) == 0){
				//the first character is an alphabet which is an issue so we should say error what
				time(&t);
				tm = localtime(&t);
				strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
				fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
				write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
			}
			else{
				
				int alreadyExists = 0;
				MessageBoxes* currentBox;
				currentBox = firstBox;
				while(currentBox != NULL){
					if(strcmp(currentBox->theBoxName, boxName) == 0){
		
						alreadyExists = 1;
						break;
					}
					if(currentBox->nextBox == NULL){
						break;
					}
					currentBox = currentBox->nextBox;
				}
				// there is no message box with this name so we send error
				if(alreadyExists != 1){
					time(&t);
					tm = localtime(&t);
					strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
					fprintf(stderr, "%s %s ER:NEXST\n", currentTime, thisDescriptor->ipAddress);
					write(thisDescriptor->descriptorNum, "ER:NEXST", 8);
				} else{
					// someone already has this box opened
					if(pthread_mutex_trylock(&currentBox->boxLocks) != 0){
						time(&t);
						tm = localtime(&t);
						strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
						fprintf(stderr, "%s %s ER:OPEND\n", currentTime, thisDescriptor->ipAddress);
						write(thisDescriptor->descriptorNum, "ER:OPEND", 8);

					}
					else{
						// you have successfully opened a box. now we will have a while loop in here for the case for an open box.
						time(&t);
						tm = localtime(&t);
						strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
						fprintf(stdout,"%s %s OPNBX\n", currentTime, thisDescriptor->ipAddress);	
						write(thisDescriptor->descriptorNum, "OK!", 3);	
						while(1){

							bzero(commands,strlen(commands));
							read(thisDescriptor->descriptorNum,commands, 6);
							//This is for closing a box inside of opened box.
							if(strcmp(commands, "CLSBX ") == 0){
								char lengthInLoop[1];
								char *boxNameInLoop = (char*) malloc(sizeof(char)*26);
								int iInLoop = 0;
								read(thisDescriptor->descriptorNum, boxNameInLoop, 26);
								while(iInLoop < 26){
									if(boxNameInLoop[iInLoop] == '\0'){
										break;
									}
									iInLoop++;
								}
								if(iInLoop == 25){
									read(thisDescriptor->descriptorNum, lengthInLoop, 1);
								}
								// it is not long enough which means this is an error what!
								if(iInLoop < 5){
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
				
								}
								//This is too long so this means error what!
								else if(iInLoop == 26){
									char readRestInLoop[1];
									read(thisDescriptor->descriptorNum, readRestInLoop, 1);
									while(readRestInLoop[0] != '\0'){
										bzero(readRestInLoop, 1);
										read(thisDescriptor->descriptorNum, readRestInLoop, 1);
									}
									read(thisDescriptor->descriptorNum, readRestInLoop, 1);
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
								}
								// the first character is not a-z which means error 
								else if(isalpha(boxNameInLoop[0]) == 0){
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
								}
								else{
									
									int alreadyExistsInLoop = 0;
									MessageBoxes* currentBoxInLoop;
									currentBoxInLoop = firstBox;
									while(currentBoxInLoop != NULL){
										if(strcmp(currentBoxInLoop->theBoxName, boxNameInLoop) == 0){
											alreadyExistsInLoop = 1;
											break;
										}
										if(currentBoxInLoop->nextBox == NULL){
											break;
										}
										currentBoxInLoop = currentBoxInLoop->nextBox;
									}
									if(alreadyExistsInLoop == 1 && strcmp(boxName,boxNameInLoop) == 0){
										alreadyExistsInLoop = 0;
										time(&t);
										tm = localtime(&t);
										strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
										fprintf(stdout,"%s %s CLSBX\n", currentTime, thisDescriptor->ipAddress);	
										write(thisDescriptor->descriptorNum, "OK!", 3);
										break;
									} else if(alreadyExistsInLoop == 1 && strcmp(boxName,boxNameInLoop) != 0){
										// you dont have the box open
										alreadyExistsInLoop = 0;
										time(&t);
										tm = localtime(&t);
										strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
										fprintf(stderr,"%s %s ER:NOOPN\n", currentTime, thisDescriptor->ipAddress);	
										write(thisDescriptor->descriptorNum, "ER:NOOPN", 8);
									}else{
										time(&t);
										tm = localtime(&t);
										strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
										fprintf(stderr, "%s %s ER:NOOPN\n", currentTime, thisDescriptor->ipAddress);
										write(thisDescriptor->descriptorNum, "ER:NOOPN", 8);
									}
								}
							} 
							else if(strncmp(commands, "NXTMG",5) == 0){
								MessageBoxes* currentBoxInLoop;
								currentBoxInLoop = firstBox;
								while(currentBoxInLoop->nextBox != NULL){
									if (strcmp(currentBoxInLoop->theBoxName, boxName) == 0){

										break;
									}
									currentBoxInLoop = currentBoxInLoop->nextBox;
									
								}
								if (currentBoxInLoop->goToMessages == NULL){
									// This is if there are no messages in here so we cant print any out
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stderr,"%s %s ER:EMPTY\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, "ER:EMPTY", 8);
								} else {
									int lenOfMessage = strlen(currentBoxInLoop->goToMessages->theMessage);
									char* easier = (char*)malloc(lenOfMessage);
									easier = currentBoxInLoop->goToMessages->theMessage;
									char* intToString = (char*)malloc(sizeof(char)*lenOfMessage);
									
									sprintf(intToString,"%d",lenOfMessage);
									int iInLoop;
									int countInLoop = 0;
									for(iInLoop = 0; iInLoop < lenOfMessage; iInLoop++){
										if (isdigit(intToString[iInLoop]) != 0){
											countInLoop ++;
										}
									}
									char* finalBufferInLoop = (char*)malloc(sizeof(char)*(lenOfMessage + countInLoop + 5));
									finalBufferInLoop[0] = 'O';
									finalBufferInLoop[1] = 'K';
									finalBufferInLoop[2] = '!';
									int startFromThree = 3;
									
									for(iInLoop = 0; iInLoop < countInLoop; iInLoop++){
										finalBufferInLoop[startFromThree] = intToString[iInLoop];
										startFromThree++;
									}
									finalBufferInLoop[startFromThree] = '!';
									startFromThree++;
									for(iInLoop = 0; iInLoop < lenOfMessage; iInLoop++){
										finalBufferInLoop[startFromThree] = easier[iInLoop];
										startFromThree++;
									}
									finalBufferInLoop[startFromThree] = '\0';
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stdout,"%s %s NXTMG\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, finalBufferInLoop, lenOfMessage + countInLoop + 5);
									Messages* prev = (Messages*)malloc(sizeof(Messages));
									prev = currentBoxInLoop->goToMessages;
									currentBoxInLoop->goToMessages = currentBoxInLoop->goToMessages->nextMessage;
									prev = NULL;
									
								}
							} 
							else if(strcmp(commands,"DELBX ") == 0){
								char lengthInLoop[1];
								char *boxNameInLoop = (char*) malloc(sizeof(char)*26);
								int iInLoop = 0;
								read(thisDescriptor->descriptorNum, boxNameInLoop, 26);
								while(iInLoop < 26){
									if(boxNameInLoop[iInLoop] == '\0'){
										break;
									}
									iInLoop++;
								}
								if(iInLoop == 25){
									read(thisDescriptor->descriptorNum, lengthInLoop, 1);
								}
								if(iInLoop < 5){
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
				
								}
								else if(iInLoop == 26){
									char readRestInLoop[1];
									read(thisDescriptor->descriptorNum, readRestInLoop, 1);
									while(readRestInLoop[0] != '\0'){
										bzero(readRestInLoop, 1);
										read(thisDescriptor->descriptorNum, readRestInLoop, 1);
									}
									read(thisDescriptor->descriptorNum, readRestInLoop, 1);
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
								}
								else if(isalpha(boxNameInLoop[0]) == 0){
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
								}
								else{
									int existsInLoop = 0;
									int countInLoop = 0;
									MessageBoxes* currentBoxInLoop;
									currentBoxInLoop = firstBox;
									MessageBoxes* prev;
									while(currentBoxInLoop != NULL){
										if(strcmp(currentBoxInLoop->theBoxName, boxNameInLoop) == 0){
											countInLoop ++;
											existsInLoop = 1;
											break;
										}
										if(currentBoxInLoop->nextBox == NULL){
											break;
										}
										prev = currentBoxInLoop;
										currentBoxInLoop = currentBoxInLoop->nextBox;
									}
									if(existsInLoop == 1 && countInLoop == 1 && currentBoxInLoop->nextBox == NULL){		
									// error the box is opened by someone else
										if(pthread_mutex_trylock(&currentBoxInLoop->boxLocks) != 0){
											time(&t);
											tm = localtime(&t);
											strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
											fprintf(stderr,"%s %s ER:OPEND\n", currentTime, thisDescriptor->ipAddress);	
											write(thisDescriptor->descriptorNum, "ER:OPEND", 8);
						
										}
										else{
											// error you can't delete because there are messages in that box still
											if(currentBoxInLoop->goToMessages != NULL){
												time(&t);
												tm = localtime(&t);
												strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
												fprintf(stderr,"%s %s ER:NOTMT\n", currentTime, thisDescriptor->ipAddress);	
												write(thisDescriptor->descriptorNum, "ER:NOTMT", 8);
												pthread_mutex_unlock(&currentBoxInLoop->boxLocks);
											}
											else if(currentBoxInLoop->goToMessages == NULL){
												firstBox = NULL;
												existsInLoop = 0;
												time(&t);
												tm = localtime(&t);
												strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
												fprintf(stdout,"%s %s DELBX\n", currentTime, thisDescriptor->ipAddress);	
												write(thisDescriptor->descriptorNum, "OK!", 3);
												pthread_mutex_unlock(&currentBoxInLoop->boxLocks);
											}
										}
									}
									else if(existsInLoop == 1 && countInLoop == 1 && currentBoxInLoop->nextBox != NULL){
										if(pthread_mutex_trylock(&currentBoxInLoop->boxLocks) != 0){
											time(&t);
											tm = localtime(&t);
											strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
											fprintf(stderr,"%s %s ER:OPEND\n", currentTime, thisDescriptor->ipAddress);	
											write(thisDescriptor->descriptorNum, "ER:OPEND", 8);
										}
										else{
											if(currentBoxInLoop->goToMessages != NULL){
												time(&t);
												tm = localtime(&t);
												strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
												fprintf(stderr,"%s %s ER:NOTMT\n", currentTime, thisDescriptor->ipAddress);	
												write(thisDescriptor->descriptorNum, "ER:NOTMT", 8);
												pthread_mutex_unlock(&currentBoxInLoop->boxLocks);
							
											}
											else if(currentBoxInLoop->goToMessages == NULL){
											// you can successfully delete the box
												firstBox = currentBoxInLoop->nextBox;
												existsInLoop = 0;
												time(&t);
												tm = localtime(&t);
												strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
												fprintf(stdout,"%s %s DELBX\n", currentTime, thisDescriptor->ipAddress);	
												write(thisDescriptor->descriptorNum, "OK!", 3);
												pthread_mutex_unlock(&currentBoxInLoop->boxLocks);
											}
										}
						
									}
									else if(existsInLoop == 1 && countInLoop != 1){
										if(pthread_mutex_trylock(&currentBoxInLoop->boxLocks) != 0){
											time(&t);
											tm = localtime(&t);
											strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
											fprintf(stderr,"%s %s ER:OPEND\n", currentTime, thisDescriptor->ipAddress);	
											write(thisDescriptor->descriptorNum, "ER:OPEND", 8);
										}
										else{
						//as long as the messages are empty we can successfully delete this box
											if(currentBoxInLoop->nextBox == NULL && currentBoxInLoop->goToMessages == NULL){
												prev->nextBox = NULL;
												time(&t);
												tm = localtime(&t);
												strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
												fprintf(stdout,"%s %s DELBX\n", currentTime, thisDescriptor->ipAddress);	
												write(thisDescriptor->descriptorNum, "OK!", 3);
												pthread_mutex_unlock(&currentBoxInLoop->boxLocks);
							
											}
											else if (currentBoxInLoop->nextBox != NULL && currentBoxInLoop->goToMessages == NULL){
												prev->nextBox = currentBoxInLoop->nextBox;
												time(&t);
												tm = localtime(&t);
												strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
												fprintf(stdout,"%s %s DELBX\n", currentTime, thisDescriptor->ipAddress);	
												write(thisDescriptor->descriptorNum, "OK!", 3);
												pthread_mutex_unlock(&currentBoxInLoop->boxLocks);
							
											}
											else if(currentBoxInLoop->nextBox == NULL && currentBoxInLoop->goToMessages != NULL){
												time(&t);
												tm = localtime(&t);
												strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
												fprintf(stderr,"%s %s ER:NOTMT\n", currentTime, thisDescriptor->ipAddress);	
												write(thisDescriptor->descriptorNum, "ER:NOTMT", 8);
												pthread_mutex_unlock(&currentBoxInLoop->boxLocks);
							
											}
											else if(currentBoxInLoop->nextBox != NULL && currentBoxInLoop->goToMessages != NULL){
												time(&t);
												tm = localtime(&t);
												strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
												fprintf(stderr,"%s %s ER:NOTMT\n", currentTime, thisDescriptor->ipAddress);	
												write(thisDescriptor->descriptorNum, "ER:NOTMT", 8);
												pthread_mutex_unlock(&currentBoxInLoop->boxLocks);
											}
						
										}
						
									}
									else{
										time(&t);
										tm = localtime(&t);
										strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
										fprintf(stderr, "%s %s ER:NEXST\n", currentTime, thisDescriptor->ipAddress);
										write(thisDescriptor->descriptorNum, "ER:NEXST", 8);
									}
					
								}
							}
							else if(strncmp(commands, "PUTMG!",6) == 0){
								//printf("Comes into put message\n");
								char* readOneBit = (char*) malloc(sizeof(char));
								read(thisDescriptor->descriptorNum, readOneBit, 1);
								
								charByChar* inLoop = (charByChar*)malloc(sizeof(charByChar));
								inLoop->oneChar = readOneBit[0];
								inLoop->nextChar = NULL;

								charByChar* ptr = (charByChar*)malloc(sizeof(charByChar));
								ptr = inLoop;
								int countInLoop = 1;
								while(1){
									bzero(readOneBit,1);
									read(thisDescriptor->descriptorNum, readOneBit, 1);
									charByChar* traverse = (charByChar*)malloc(sizeof(charByChar));
									traverse->oneChar = readOneBit[0];

									traverse->nextChar = NULL;
									if (isdigit(traverse->oneChar) != 0){
										countInLoop++;
									}
									traverse->nextChar = NULL;
									while(ptr->nextChar != NULL){
										ptr = ptr->nextChar;
									}
									ptr->nextChar = traverse;
									if (isdigit(traverse->oneChar) == 0){
										break;
									}
								}
								if (ptr->nextChar->oneChar != '!'){
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
								} else{

									charByChar* loopOver = (charByChar*)malloc(sizeof(charByChar));
									loopOver = inLoop;
									char* stringToInt = (char*)malloc(countInLoop);
									int iInLoop;
									for(iInLoop = 0; iInLoop < countInLoop; iInLoop++){
										stringToInt[iInLoop] = loopOver->oneChar;

										loopOver = loopOver->nextChar;
									}
									int numInLoop = 0;
									numInLoop = atoi(stringToInt);
									//printf("Numbers to read is: %d\n",numInLoop);
									char* readRestInLoop = (char*)malloc(numInLoop+1);
									readRestInLoop[numInLoop] = '\0';
									read(thisDescriptor->descriptorNum, readRestInLoop, numInLoop);
									//printf("Now We Have %s\n",readRestInLoop);
									MessageBoxes* boxInLoop;
									boxInLoop = firstBox;
									
									while(strcmp(boxInLoop->theBoxName,boxName) != 0){
										
										boxInLoop = boxInLoop->nextBox;
										
									}
									//printf("Now We Are Comparing: %s\n",boxInLoop->theBoxName);
									if(boxInLoop->goToMessages == NULL){
										//printf("Yes\n");
									}
									if(boxInLoop->goToMessages == NULL){
										boxInLoop->goToMessages = (Messages*)malloc(sizeof(Messages));
										boxInLoop->goToMessages->theMessage = (char*)malloc(numInLoop);
										boxInLoop->goToMessages->theMessage = readRestInLoop;
										//printf("Now We Have: %s\n",boxInLoop->goToMessages->theMessage );
										boxInLoop->goToMessages->nextMessage = NULL;
									} else{
										Messages* messageInLoop;
										messageInLoop = boxInLoop->goToMessages;
										while(messageInLoop->nextMessage != NULL){
											messageInLoop = messageInLoop->nextMessage;
										}
										messageInLoop->nextMessage = (Messages*)malloc(sizeof(Messages));
										messageInLoop->nextMessage->theMessage = (char*)malloc(numInLoop);
										messageInLoop->nextMessage->theMessage = readRestInLoop;
										messageInLoop->nextMessage->nextMessage = NULL;
									}
									char* finalAnsToReturn = malloc(4+countInLoop);
									finalAnsToReturn[0] = 'O';
									finalAnsToReturn[1] = 'K';
									finalAnsToReturn[2] = '!';
									int finalI = 3;
									for(iInLoop = 0; iInLoop < countInLoop; iInLoop ++){
										finalAnsToReturn[finalI] = stringToInt[iInLoop];
										finalI++;
									}
									finalAnsToReturn[finalI] = '\0';
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stdout,"%s %s PUTMG\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, finalAnsToReturn, 6+countInLoop);
									//printf("We've sent %s\n",finalAnsToReturn);
									char whatEver[1];
									read(thisDescriptor->descriptorNum, whatEver, 1);
								}
							}
							else if(strcmp(commands, "OPNBX ") == 0){
								char* readOne = (char*) malloc(sizeof(char));
								time(&t);
								tm = localtime(&t);
								strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
								fprintf(stderr,"%s %s ER:EXCRD\n", currentTime, thisDescriptor->ipAddress);	
								write(thisDescriptor->descriptorNum, "ER:EXCRD", 8);
								read(thisDescriptor->descriptorNum, readOne, 1);
								while(readOne[0] != '\0'){
									read(thisDescriptor->descriptorNum, readOne, 1);
								}
								read(thisDescriptor->descriptorNum, readOne, 1);
								
								
								
								
							}
							else if(strcmp(commands, "CREAT ") == 0 ){
											
								char length[1];
								char *boxNameInLoop = (char*) malloc(sizeof(char)*26);
								int i = 0;
								//bzero(boxName, strlen(boxName));
								read(thisDescriptor->descriptorNum, boxNameInLoop, 26);
								while(i < 26){

									if(boxNameInLoop[i] == '\0'){
										break;
									}
									i++;
								}


								if(i == 25){
									read(thisDescriptor->descriptorNum, length, 1);
								}

								// length is less than 5 should be an error
								if(i < 5){
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
				
								}
								// length equals 26 means this is an error since it is longer than 25 characters
								else if(i == 26){
									char readRest[1];
									read(thisDescriptor->descriptorNum, readRest, 1);
									while(readRest[0] != '\0'){
										bzero(readRest, 1);
										read(thisDescriptor->descriptorNum, readRest, 1);
									}
									read(thisDescriptor->descriptorNum, readRest, 1);
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
								}
								// the first character is a number which should never be the case. it should always start with an alphabet
								else if(isalpha(boxNameInLoop[0]) == 0){
									time(&t);
									tm = localtime(&t);
									strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
									fprintf(stderr,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);	
									write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
								}
								else{
				
									pthread_mutex_lock(&createLock);
									if(firstBox == NULL){
										//you should create the first message box
										MessageBoxes* holderBoxInLoop = (MessageBoxes*) malloc(sizeof(MessageBoxes));
										holderBoxInLoop->theBoxName = (char*) malloc(sizeof(char) * 26);
										holderBoxInLoop->theBoxName = boxNameInLoop;
										holderBoxInLoop->nextBox = NULL;
										holderBoxInLoop->goToMessages = NULL;
										holderBoxInLoop->boxLocks = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
										firstBox = holderBoxInLoop;
										time(&t);
										tm = localtime(&t);
										strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
										fprintf(stdout, "%s %s CREAT\n", currentTime, thisDescriptor->ipAddress); // the first box was successfully created
										write(thisDescriptor->descriptorNum, "OK!", 3);
									}
									else{

										int alreadyExists = 0;
										MessageBoxes* currentBoxInLoop;
										currentBoxInLoop = firstBox;

										while(currentBoxInLoop != NULL){
											//printf("currentbox name is\n: %s", currentBox->theBoxName);
											//printf("we are comparing: %s and %s\n", currentBox->nextBox->theBoxName, boxName);
											if(strcmp(currentBoxInLoop->theBoxName, boxNameInLoop) == 0){
												alreadyExists = 1;
												break;
											}
											if(currentBoxInLoop->nextBox == NULL){
												break;
											}
											currentBoxInLoop = currentBoxInLoop->nextBox;
										}
										// you are trying to enter in a box that already exists. should be an error
										if(alreadyExists == 1){
											alreadyExists = 0;
											time(&t);
											tm = localtime(&t);
											strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
											fprintf(stderr,"%s %s ER:EXIST\n", currentTime, thisDescriptor->ipAddress);	
											write(thisDescriptor->descriptorNum, "ER:EXIST", 8);
						
										}
										else{
											//the box does not exist and it can be successfully created!
											currentBoxInLoop->nextBox = (MessageBoxes*) malloc(sizeof(MessageBoxes));
											currentBoxInLoop = currentBox->nextBox;
											currentBoxInLoop->theBoxName = (char*) malloc(sizeof(char) * 26);
											currentBoxInLoop->theBoxName = boxNameInLoop;
											currentBoxInLoop->goToMessages = NULL;
											currentBoxInLoop->nextBox = NULL;
											currentBoxInLoop->boxLocks = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
											time(&t);
											tm = localtime(&t);
											strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
											fprintf(stdout, "%s %s CREAT\n", currentTime, thisDescriptor->ipAddress);
											write(thisDescriptor->descriptorNum, "OK!", 3);
										}
					
					
									}
									pthread_mutex_unlock(&createLock);
			
								}
								
								
							}
							else if(strcmp(commands, "GDBYE") == 0 ){
								//printf("Reached here");
								time(&t);
								tm = localtime(&t);
								write(thisDescriptor->descriptorNum, "\0", 1);
								strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
								fprintf(stdout,"%s %s disconneted\n", currentTime, thisDescriptor->ipAddress);
								pthread_mutex_unlock(&currentBox->boxLocks);
								pthread_exit(0);
							}
							else{
								//printf("comes in here\n");
								// otherwise this is an error
								char readRest[1];
								read(thisDescriptor->descriptorNum, readRest, 1);

								while(readRest[0] != '\0'){
									bzero(readRest, 1);
									read(thisDescriptor->descriptorNum, readRest, 1);
									//printf("we are now at %c\n",readRest[0]);
								}
								time(&t);
								tm = localtime(&t);
								strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
								fprintf(stdout,"%s %s ER:WHAT?\n", currentTime, thisDescriptor->ipAddress);
								write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
								read(thisDescriptor->descriptorNum, readRest, 1);
							}
						}		
					pthread_mutex_unlock(&currentBox->boxLocks);	
					}
				
				}
			}

		}
		else if(strncmp(commands, "CLSBX ",6) == 0){
			char readRest[1];
			read(thisDescriptor->descriptorNum, readRest, 1);
			//printf("we are now at %c\n",readRest[0]);
			while(readRest[0] != '\0'){
				bzero(readRest, 1);
				read(thisDescriptor->descriptorNum, readRest, 1);
				//printf("we are now at %c\n",readRest[0]);
			}
			read(thisDescriptor->descriptorNum, readRest, 1);
			time(&t);
			tm = localtime(&t);
			strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
			fprintf(stderr, "%s %s ER:NOOPN\n", currentTime, thisDescriptor->ipAddress);
			write(thisDescriptor->descriptorNum, "ER:NOOPN", 8);
		}
		else if(strncmp(commands, "NXTMG",5) == 0){
			time(&t);
			tm = localtime(&t);
			strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
			fprintf(stderr, "%s %s ER:NOOPN\n", currentTime, thisDescriptor->ipAddress);
			write(thisDescriptor->descriptorNum, "ER:NOOPN", 8);
		}
		else if(strncmp(commands, "PUTMG!",6) == 0){
			char readRest[1];
			read(thisDescriptor->descriptorNum, readRest, 1);
			//printf("we are now at %c\n",readRest[0]);
			while(readRest[0] != '\0'){
				bzero(readRest, 1);
				read(thisDescriptor->descriptorNum, readRest, 1);
				//printf("we are now at %c\n",readRest[0]);
			}
			//read(thisDescriptor->descriptorNum, readRest, 1);
			time(&t);
			tm = localtime(&t);
			strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
			fprintf(stderr, "%s %s ER:NOOPN\n", currentTime, thisDescriptor->ipAddress);
			//printf("Reached\n");
			write(thisDescriptor->descriptorNum, "ER:NOOPN", 8);
			
		}
		else{
			strftime(currentTime, sizeof(currentTime), "%k%M %d %b", tm);
			fprintf(stderr,"%s %s ER:WHAT?", currentTime, thisDescriptor->ipAddress);
			write(thisDescriptor->descriptorNum, "ER:WHAT?", 8);
		}
	}

	pthread_exit(0);
	
	
	
}




int main(int argc, char ** argv){

	//printf("Server\n");

	if(argc < 2){
		printf("Error. Not enough Arguments\n");
		return 0;
	}
	if(argc > 2){
		printf("Error. Too many Arguments\n");
		return 0;
	}
	//Get the port number from the command line
	int thePortNumber = atoi(argv[1]);
	//printf("the port number is: %d\n", thePortNumber);

	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(thePortNumber);
	int bindReturns = bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));
	
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);

	listen(server_socket, 30);
	firstBox = NULL;


	while(1){
		Descriptors* currentDescriptor = (Descriptors*) malloc(sizeof(Descriptors));
		pthread_t createClientThread;
		currentDescriptor->descriptorNum = accept(server_socket, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		currentDescriptor->ipAddress = malloc(sizeof(char)*25);
		currentDescriptor->ipAddress = inet_ntoa(clnt_addr.sin_addr);
		pthread_create(&createClientThread, NULL, talkToClient, (void*)currentDescriptor);

	}


	return 0;
}