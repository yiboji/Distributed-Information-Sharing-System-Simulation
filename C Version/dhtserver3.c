#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define TRUE 1
#define FALSE 0

#define PORT "23887"
#define BACKLOG 10
#define MAXBUFFLEN 100

struct Table{
	char *key;
	char *value;
};
struct Table table[50];
//read txt file and store into a struct Table array
int readFile(struct Table *p, char *fileName){
	FILE *fp;
	int index = 0;
	char strIn[20]; 
	fp=fopen(fileName, "r");
	if(fp==NULL){
		perror("Error on opening file");
		return -1;
	}
//read from file, and compare key and value;
	while(fgets(strIn,20, fp)!=NULL){
		p->key = malloc(10*sizeof(char));
		p->value = malloc(10*sizeof(char));
		strcpy(p->key , strtok(strIn, " "));
		strcpy(p->value, strtok(NULL, "\n"));
		index++;	
		p++;
	}
	fclose(fp);
	return index; //return the size of table;
}
//search table for value by key
char* searchTable(struct Table *p, char *searchStr, int tableSize){
	int i = 0;
	char *matchStr;
	for(i=0; i<tableSize; i++){
		if(!strcmp(p->key, searchStr)){
			matchStr = malloc(10*sizeof(char));
			strcpy(matchStr, p->value);	
			return matchStr;
		}
		p++;
	}
	return NULL;
}
//remove "GET" or "POST" from message
void removeCommand(char *dest, char *src){
	char tempStr[MAXBUFFLEN];
	strcpy(tempStr, src);

	strtok(tempStr, " ");
	strcpy(dest, strtok(NULL, "\0"));	
}
//add "GET" or "POST" for message
void addCommand(char *dest, char *src, char *command){
	strcpy(dest, command);
	strcat(dest, " ");
	strcat(dest, src);
}
//main function
int main(void){

	int sockfd, childSockfd;
	struct addrinfo localHost, *p;
	struct sockaddr remotAddr;
	
	socklen_t sin_size = sizeof remotAddr;
	char str[INET_ADDRSTRLEN];
	int rv;
	char recvBuf[MAXBUFFLEN];
	char *matchValue;
	char *keyValue;
	char sendBuf[MAXBUFFLEN];

	char *localIPStr;
	char clientTCPIPStr[INET_ADDRSTRLEN];
	int clientTCPPortNum;
    
    int tableSize = readFile(table, "server3.txt");
    

	memset(&localHost, 0, sizeof localHost);
	localHost.ai_family = AF_INET;
	localHost.ai_socktype = SOCK_STREAM;
	localHost.ai_flags = AI_PASSIVE;
//initiate TCP connection	
	if(getaddrinfo(NULL, PORT, &localHost, &p)!=0){
		printf("get address information in error!\n");
		return 0;
	}
	
	sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	if(sockfd==-1){
		perror("server: socket");	
		return 0;
	}
	if(bind(sockfd, p->ai_addr, p->ai_addrlen)==-1){
		close(sockfd);
		perror("server2: bind");
		return 0;
	}

	while(1){
		if(listen(sockfd, BACKLOG)==-1){
			perror("listen");
			exit(1);
		}

//show TCP port number and IP address of server3
		{
		struct sockaddr localAddr;
		int addr_len = sizeof localAddr;
		struct sockaddr_in *local_p;

		getsockname(sockfd, &localAddr, &addr_len);
		local_p = (struct sockaddr_in*)&localAddr;
	    struct hostent *h = gethostbyname("localhost");     
	    localIPStr = inet_ntoa(*((struct in_addr*)(h->h_addr)));
		printf("The Server 3 has TCP port number %s and IP address %s.\n",PORT, localIPStr); 
	
		}
//accept server2
		childSockfd = accept(sockfd, &remotAddr, &sin_size);	
		if(childSockfd==-1){
			perror("accept");
			return 0;
		}
//TCP receive from server2	
		if(recv(childSockfd, recvBuf, MAXBUFFLEN, 0)==-1){
			perror("recv\n");
			return 0;
		}
//search table
	    keyValue = malloc(10*sizeof(char));
	    removeCommand(keyValue, recvBuf);
	    matchValue = searchTable(table, keyValue, tableSize);

//show key value and port number&IP address of server2
		{
		struct sockaddr_in *remoteAddr_p;
		remoteAddr_p = (struct sockaddr_in *)&remotAddr;
	    clientTCPPortNum = (int)ntohs(remoteAddr_p->sin_port);
	    strcpy(clientTCPIPStr,  inet_ntoa(remoteAddr_p->sin_addr));
		printf("The Server 3 has received a request with key %s from the Server 2 with port number %d and IP address %s.\n", recvBuf, clientTCPPortNum, clientTCPIPStr);
		}
		
	
		if(matchValue!=NULL){
			addCommand(sendBuf, matchValue, "POST");
			printf("The Server 3 sends the reply %s to the Server 2 with port number %d and IP address %s.\n", sendBuf, clientTCPPortNum,clientTCPIPStr); 
			if(send(childSockfd, sendBuf, strlen(sendBuf)+1, 0)==-1){
				perror("server3: send back in error\n");
				return 0;	
			}
		}
		else{
		addCommand(sendBuf, matchValue, "POST");
		send(childSockfd, sendBuf, strlen(sendBuf)+1, 0);
			printf("server3: can not find value for key\n");
		}

		close(childSockfd);
	}	
	close(sockfd);

	return 0;
}
