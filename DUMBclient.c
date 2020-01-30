#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>

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


int main(int argc, char ** argv){

	if(argc < 3){
		
		printf("Error. There are not enough arguments.\n");
		return 0;
		
	}
	if(argc > 3){
		printf("Error. There are not enough arguments.\n");
		return 0;
	}

int thePortNumber;
thePortNumber = atoi(argv[2]);
int client_socket;
client_socket = socket(AF_INET, SOCK_STREAM, 0);

struct sockaddr_in server_addr;
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(thePortNumber);

struct hostent *website_entered;
char * getIp;

website_entered = gethostbyname(argv[1]);
getIp = inet_ntoa(*((struct in_addr*)website_entered->h_addr));   
server_addr.sin_addr.s_addr = inet_addr(getIp);  

int didItConnect = connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));
char* buffer;
char helloServer[255];
helloServer[19] = '\0';
int countConnection = 1;
//if the connection fails, try two more times.
while (didItConnect == -1){
	if (countConnection == 3){
		break;	
	}
	didItConnect = connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));
	countConnection++;
	
}
if (countConnection == 3){
	printf("ERROR ON CONNECTION\n");
	return 0;
}
if (didItConnect != -1){
	write(client_socket,"HELLO",5);
	read(client_socket,helloServer,255);
	if (strncmp(helloServer, "HELLO DUMBv0 ready!",19) != 0){
		printf("ERROR ON CONNECTION\n");
		return 0;
	}
	//Connection succeeds
	fprintf(stdout,helloServer);
	fprintf(stdout,"\n");
}


while (1){
	int n = 0;
		fprintf(stdout,"> ");
		scanf("%m[^'\n']",&buffer);
		char ch;
		while((ch = getchar()) != '\n'&& ch!= EOF);
		if (strncmp("help", buffer,4) == 0){
			fprintf(stdout,"Try one of the following commands.\n");
			fprintf(stdout,"quit\n");
			fprintf(stdout,"create\n");
			fprintf(stdout,"delete\n");
			fprintf(stdout,"open\n");
			fprintf(stdout,"close\n");
			fprintf(stdout,"next\n");
			fprintf(stdout,"put\n");
		}
		else if (strncmp("quit", buffer,4) == 0){
			bzero(buffer,strlen(buffer));
			write(client_socket,"GDBYE",5);
			//int if_quit;
			//if_quit = connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));
			char readToCheck[1];
			read(client_socket, readToCheck,1);
			if(readToCheck[0] == '\0'){
				fprintf(stdout,"Shut down successfully.\n");
				break;
			} else{
				read(client_socket,buffer,255);
				fprintf(stderr,"Failed to shut down.\n");
			}
		}
		else if (strncmp("create", buffer,6) == 0){
			fprintf(stdout,"Ok, create a message box that is 5 to 25 characters long, and start with an alphabetic character.\n");
			bzero(buffer,strlen(buffer));
			fprintf(stdout,"create:> ");
			scanf("%m[^'\n']",&buffer);
			//we are reading from user
			char* storeMessage = (char*)malloc(sizeof(char)*(strlen(buffer)+1));
			storeMessage[strlen(buffer)] = '\0';
			int goThrough = 0;
			for(goThrough = 0; goThrough < strlen(buffer); goThrough++){
				storeMessage[goThrough] = buffer[goThrough];
			}
			char ch;
			while((ch = getchar()) != '\n'&& ch!= EOF);
			int length = strlen(buffer) + 7;
			char newBuffer[length + 1];
			int clean = 0;
			for(clean = 0; clean < length+1; clean++){
				newBuffer[clean] = '\0';
			}
			int i;
			//we are creating newbuffer, and this will be sent to server
			newBuffer[0] = 'C';
			newBuffer[1] = 'R';
			newBuffer[2] = 'E';
			newBuffer[3] = 'A';
			newBuffer[4] = 'T';
			newBuffer[5] = ' ';
			int command = 6;
			for(i = 0; i < length; i++){
				newBuffer[command] = buffer[i];
				command ++;
			}
			newBuffer[command] = '\0';
			write(client_socket,newBuffer,length+1);
			bzero(buffer,strlen(buffer));
			char* serverMessage = (char*) malloc(sizeof(char) * 256);
			int cleanIt = 0;
			for(cleanIt = 0; cleanIt < 256; cleanIt++){
				serverMessage[cleanIt] = '\0';
			}
			read(client_socket,serverMessage,255);
			// read from server, and tell user what is going on.
			if(strcmp(serverMessage,"OK!") == 0){
				printf("%s\n",serverMessage);
				printf("Success! Message box '%s' is now created.\n",storeMessage);
			}else if(strcmp(serverMessage,"ER:WHAT?") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Your message is in some way broken or malformed, please try again.\n");
			}else if(strcmp(serverMessage,"ER:EXIST") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Message box '%s' already exists.\n",storeMessage);
			}
		}
		else if (strncmp("delete", buffer,6) == 0){
			fprintf(stdout,"Ok, delete a message box that you have created.\n");
			bzero(buffer,strlen(buffer));
			fprintf(stdout,"delete:> ");
			scanf("%m[^'\n']",&buffer);
			// read from user
			char* storeMessage = (char*)malloc(sizeof(char)*(strlen(buffer)+1));
			storeMessage[strlen(buffer)] = '\0';
			int goThrough = 0;
			for(goThrough = 0; goThrough < strlen(buffer); goThrough++){
				storeMessage[goThrough] = buffer[goThrough];
			}
			char ch;
			while((ch = getchar()) != '\n'&& ch!= EOF);
			int length = strlen(buffer) + 7;
			char newBuffer[length + 1];
			int i;
			// create newBuffer that is going to be sent to server.
			newBuffer[0] = 'D';
			newBuffer[1] = 'E';
			newBuffer[2] = 'L';
			newBuffer[3] = 'B';
			newBuffer[4] = 'X';
			newBuffer[5] = ' ';
			int command = 6;
			for(i = 0; i < length; i++){
				newBuffer[command] = buffer[i];
				command ++;
			}
			newBuffer[command] = '\0';
			write(client_socket,newBuffer,length+1);
			bzero(buffer,strlen(buffer));
			char* serverMessage = (char*) malloc(sizeof(char) * 256);
			int cleanIt = 0;
			for(cleanIt = 0; cleanIt < 256; cleanIt++){
				serverMessage[cleanIt] = '\0';
			}
			read(client_socket,serverMessage,255);
			//read from server and tell user what is going on.
			if(strcmp(serverMessage,"OK!") == 0){
				printf("%s\n",serverMessage);
				printf("Success! Message box '%s' is now deleted.\n",storeMessage);
			}else if(strcmp(serverMessage,"ER:NEXST") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Message box '%s' does not exist.\n",storeMessage);
			}else if(strcmp(serverMessage,"ER:OPEND") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Message box '%s' is currently opned.\n",storeMessage);
			}else if(strcmp(serverMessage,"ER:NOTMT") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Message box '%s' is not empty.\n",storeMessage);
			}else if(strcmp(serverMessage,"ER:WHAT?") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Your message is in some way broken or malformed, please try again.\n");
			}
		}
		else if (strncmp("open", buffer,4) == 0){
			fprintf(stdout,"Ok, open a message box that is you have created.\n");
			bzero(buffer,strlen(buffer));
			fprintf(stdout,"open:> ");
			scanf("%m[^'\n']",&buffer);
			// read from user
			char* storeMessage = (char*)malloc(sizeof(char)*(strlen(buffer)+1));
			storeMessage[strlen(buffer)] = '\0';
			int goThrough = 0;
			for(goThrough = 0; goThrough < strlen(buffer); goThrough++){
				storeMessage[goThrough] = buffer[goThrough];
			}
			char ch;
			while((ch = getchar()) != '\n'&& ch!= EOF);
			int length = strlen(buffer) + 7;
			char newBuffer[length + 1];
			int i;
			// write newBuffer that is going to be sent to server
			newBuffer[0] = 'O';
			newBuffer[1] = 'P';
			newBuffer[2] = 'N';
			newBuffer[3] = 'B';
			newBuffer[4] = 'X';
			newBuffer[5] = ' ';
			int command = 6;
			for(i = 0; i < length; i++){
				newBuffer[command] = buffer[i];
				command ++;
			}
			newBuffer[command] = '\0';
			write(client_socket,newBuffer,length+1);
			bzero(buffer,strlen(buffer));
			char* serverMessage = (char*) malloc(sizeof(char) * 256);
			int cleanIt = 0;
			for(cleanIt = 0; cleanIt < 256; cleanIt++){
				serverMessage[cleanIt] = '\0';
			}
			read(client_socket,serverMessage,255);
			// read from server and tell user what is going on
			if(strcmp(serverMessage,"OK!") == 0){
				printf("%s\n",serverMessage);
				printf("Success! Message box '%s' is now opened.\n",storeMessage);
			}else if(strcmp(serverMessage,"ER:NEXST") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Message box '%s' does not exist.\n",storeMessage);
			}else if(strcmp(serverMessage,"ER:OPEND") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Message box '%s' is currently opned.\n",storeMessage);
			}else if(strcmp(serverMessage,"ER:WHAT?") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Your message is in some way broken or malformed, please try again.\n");
			}else if(strcmp(serverMessage,"ER:EXCRD") == 0){
				printf("%s\n",serverMessage);
				printf("Error. You are trying to open another box while having one open.\n");
			}
		}
		else if (strncmp("close", buffer,5) == 0){
			fprintf(stdout,"Ok, close a message box that you have created.\n");
			bzero(buffer,strlen(buffer));
			fprintf(stdout,"close:> ");
			scanf("%m[^'\n']",&buffer);
			// read from user
			char* storeMessage = (char*)malloc(sizeof(char)*(strlen(buffer)+1));
			storeMessage[strlen(buffer)] = '\0';
			int goThrough = 0;
			for(goThrough = 0; goThrough < strlen(buffer); goThrough++){
				storeMessage[goThrough] = buffer[goThrough];
			}
			char ch;
			while((ch = getchar()) != '\n'&& ch!= EOF);
			int length = strlen(buffer) + 7;
			char newBuffer[length + 1];
			int i;
			// create newbuffer that is going to be sent to server
			newBuffer[0] = 'C';
			newBuffer[1] = 'L';
			newBuffer[2] = 'S';
			newBuffer[3] = 'B';
			newBuffer[4] = 'X';
			newBuffer[5] = ' ';
			int command = 6;
			for(i = 0; i < length; i++){
				newBuffer[command] = buffer[i];
				command ++;
			}
			newBuffer[command] = '\0';
			write(client_socket,newBuffer,length+1);
			bzero(buffer,strlen(buffer));
			char* serverMessage = (char*) malloc(sizeof(char) * 256);
			int cleanIt = 0;
			for(cleanIt = 0; cleanIt < 256; cleanIt++){
				serverMessage[cleanIt] = '\0';
			}
			read(client_socket,serverMessage,255);
			// read from server and tell user what is going on.
			if(strcmp(serverMessage,"OK!") == 0){
				printf("%s\n",serverMessage);
				printf("Success! Message box '%s' is now closed.\n",storeMessage);
			}else if(strcmp(serverMessage,"ER:NOOPN") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Message box '%s' is currently not opened so can't be closed.\n",storeMessage);
			}else if(strcmp(serverMessage,"ER:WHAT?") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Your message is in some way broken or malformed, please try again.\n");
			}
		}
		else if (strncmp("next", buffer,4) == 0){
			write(client_socket,"NXTMG",5);
			bzero(buffer,strlen(buffer));
			read(client_socket,buffer,3);
			charByChar* bufferNext = (charByChar*)malloc(sizeof(charByChar));
			// don't require user input, fetch data from server.
			if((strncmp("OK!", buffer,3)) == 0){
				// if the frist 3 characters are "OK!", then try to read what is the message, according to
				// the numbers in the message server sent
				char readThrough[1];
				read(client_socket,readThrough,1);
				charByChar* travers = (charByChar*)malloc(sizeof(charByChar));
				travers->oneChar = readThrough[0];
				travers->nextChar = NULL;
			
				bufferNext = travers;
				charByChar* loopThrough = (charByChar*)malloc(sizeof(charByChar));
				loopThrough = bufferNext;
				int count = 1;
				while (1){
					bzero(readThrough,1);
					read(client_socket,readThrough,1);
					charByChar* inLoop = (charByChar*)malloc(sizeof(charByChar));
					inLoop->oneChar = readThrough[0];
					inLoop->nextChar = NULL;
					while(loopThrough->nextChar != NULL){
						loopThrough = loopThrough->nextChar;
					}
					loopThrough->nextChar = inLoop;
					
					if (readThrough[0] != '!'){
						count ++;
					} else if (readThrough[0] == '!'){
						break;
					}

				}
				
				charByChar* checkLoop = (charByChar*)malloc(sizeof(charByChar));
				checkLoop = bufferNext;
				
				char intToString[count];
				int i = 0;
				while(checkLoop != NULL){
					intToString[i] = checkLoop->oneChar;
					checkLoop = checkLoop->nextChar;
					i++;
				}
				int num = atoi(intToString);
				char* readRest = (char*)malloc(num+1);
				readRest[num] = '\0';
				read(client_socket,readRest,num+1);
				char* finalBuffer = (char*)malloc(num+1+4+count);
				finalBuffer[0] = 'O';
				finalBuffer[1] = 'K';
				finalBuffer[2] = '!';
				int okCommand = 3;
				for(i = 0; i < count; i++){
					finalBuffer[okCommand] = intToString[i];
					okCommand++;
				}
				finalBuffer[okCommand] = '!';
				okCommand++;
				for(i = 0; i < num; i++){
					finalBuffer[okCommand] = readRest[i];
					//printf("rest is: %c\n",readRest[i]);
					okCommand++;
				}
				finalBuffer[okCommand] = '\0';
				printf(finalBuffer);
				printf("\n");
				printf("Success! Successfully fetched message from message box.\n");
			}else {
				// if it's not "OK!", then it's the error case
				char* finalBuffer = (char*)malloc(255);
				char* realFinalBuffer = (char*)malloc(255);
				int n; int i;
				n = read(client_socket,finalBuffer,255);
				realFinalBuffer[0] = 'E';
				realFinalBuffer[1] = 'R';
				realFinalBuffer[2] = ':';
				int erCommand = 3;
				for(i = 0; i < n; i++){
					realFinalBuffer[erCommand] = finalBuffer[i];
					erCommand++;
				}
				if(strcmp(realFinalBuffer,"ER:NOOPN") == 0){
					printf("%s\n",realFinalBuffer);
					printf("Error. No message box is opened.\n");
				}else if(strcmp(realFinalBuffer,"ER:EMPTY") == 0){
					printf("%s\n",realFinalBuffer);
					printf("Error. No Message left in the message box.\n");
				}else if(strcmp(realFinalBuffer,"ER:WHAT?") == 0){
					printf("%s\n",realFinalBuffer);
					printf("Error. Your message is in some way broken or malformed, please try again.\n");
				}
			}
	
			
		}
		else if (strncmp("put", buffer,3) == 0){
			
			fprintf(stdout,"Ok, put what you want into the message box.\n");
			bzero(buffer,strlen(buffer));
			fprintf(stdout,"put:> ");
			scanf("%m[^'\n']",&buffer);
			// read from client
			char* storeMessage = (char*)malloc(sizeof(char)*(strlen(buffer)+1));
			storeMessage[strlen(buffer)] = '\0';
			int goThrough = 0;
			for(goThrough = 0; goThrough < strlen(buffer); goThrough++){
				storeMessage[goThrough] = buffer[goThrough];
			}
			//printf("The thing we entered is %s\n",buffer);
			char ch;
			while((ch = getchar()) != '\n'&& ch!= EOF);
			int length = strlen(buffer);
			char* intToString = (char*)malloc(sizeof(char)*length);
		
			sprintf(intToString,"%d",length);
			int i;
			int count = 0;
			for(i = 0; i < length; i++){
				if (isdigit(intToString[i]) != 0){
					count ++;
				}
			}
			//printf("The length is %d\n",count);
			// prepare newbuffer that is going to be sent to server with the message user
			// wants to put in the box
			char newBuffer[length + count + 8];
			newBuffer[0] = 'P';
			newBuffer[1] = 'U';
			newBuffer[2] = 'T';
			newBuffer[3] = 'M';
			newBuffer[4] = 'G';
			newBuffer[5] = '!';
			int c = 6;
			for(i = 0; i < count; i ++){
				newBuffer[c] = intToString[i];
				c ++;
			}
			newBuffer[c] = '!';
			//printf("Check here: %c\n",newBuffer[c]);
			c = c + 1;
			//printf("Command is: %d\n",c);
			for(i = 0; i < length; i++){
				newBuffer[c] = buffer[i];
				//printf("We are checking %c\n",newBuffer[c]);
				c ++;
			}
			
			newBuffer[c] = '\0';
			
			write(client_socket,newBuffer,strlen(newBuffer)+1);
			//printf("The Thing we want to send is %s\n",newBuffer);
			bzero(buffer,strlen(buffer));
			char* serverMessage = (char*) malloc(sizeof(char) * 256);
			int cleanIt = 0;
			for(cleanIt = 0; cleanIt < 256; cleanIt++){
				serverMessage[cleanIt] = '\0';
			}
			read(client_socket,serverMessage,255);
			if(strcmp(serverMessage,"ER:NOOPN") == 0){
				printf("%s\n",serverMessage);
				printf("Error. No message box is opened.\n");
			}else if(strcmp(serverMessage,"ER:WHAT?") == 0){
				printf("%s\n",serverMessage);
				printf("Error. Your message is in some way broken or malformed, please try again.\n");
			}else{
				printf("%s\n",serverMessage);
				printf("Success! Message '%s' is now put into the box.\n",storeMessage);
			}
			
		}
		else{
			printf("That is not a command, for a command list enter 'help'.\n");
		}
		

	}



	return 0;

}