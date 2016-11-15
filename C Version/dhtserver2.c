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

#define PORT "22887"
#define BACKLOG 10
#define MAXBUFFLEN 100
#define SERVER3_PORT "23887"

char *matchValue;
char *keyValue;

char sendBuf[MAXBUFFLEN];
char recvBuf[MAXBUFFLEN];

char *localIPStr;
char clientTCPIPStr[INET_ADDRSTRLEN];
int clientTCPPortNum;

char server3SendBuf[MAXBUFFLEN];
int  localTCPPortNum;

struct Table{
	char *key;
	char *value;
};
struct Table table[50];
//read file and store in struct Table array
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
//remove "POST" and "GET" from message
void removeCommand(char *dest, char *src){
	char tempStr[MAXBUFFLEN];
	strcpy(tempStr, src);

	strtok(tempStr, " ");
	strcpy(dest, strtok(NULL, "\0"));	
}
//add "POST" and "GET" for message
void addCommand(char *dest, char *src, char *command){
	strcpy(dest, command);
	strcat(dest, " ");
	strcat(dest, src);
}
//ask next server for value
char* askNextServer(char *serverPort, char *requst){

	int TCPSockfd;
	struct addrinfo serverAddr, *p;
	int numBytes;
	memset(&serverAddr, 0, sizeof serverAddr);
	serverAddr.ai_family = AF_INET;
	serverAddr.ai_socktype = SOCK_STREAM;
		
	if(getaddrinfo("localhost", serverPort, &serverAddr, &p)==-1){
		printf("get address infomation in error!\n");
		return NULL;	
	}
	TCPSockfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	if(TCPSockfd==-1){
		perror("server1: client: socket:");
		return NULL;
	}
	if(connect(TCPSockfd, p->ai_addr, p->ai_addrlen)==-1){
		perror("server1: TCP client: connect");
		return NULL;
	}
//show TCP port number and IP address of server2
	struct sockaddr localAddr;
	int addr_len = sizeof localAddr;
	struct sockaddr_in *local_p;
	getsockname(TCPSockfd, &localAddr, &addr_len);
	local_p = (struct sockaddr_in*)&localAddr;
	localTCPPortNum = ntohs(local_p->sin_port);
	printf("The Server 2 sends the request %s to the Server3.\n",  recvBuf);	 
	printf("The TCP port number is %d and the IP address is %s.\n", localTCPPortNum, localIPStr);
//TCP forword packet	
	if(send(TCPSockfd, requst, strlen(requst)+1, 0)==-1){
		perror("send");
		return 0;
	}
    char recvBuf[MAXBUFFLEN];
	if(recv(TCPSockfd, recvBuf, MAXBUFFLEN, 0)==-1){
		perror("server1: receive from server2 in error\n");
		return 0;
	} 
//show value and port number&IP address of server2
	struct sockaddr remoteAddr;
	struct sockaddr_in *remoteAddr_p;
    char server3IPStr[INET_ADDRSTRLEN];
	addr_len = sizeof(remoteAddr);

	getpeername(TCPSockfd, &remoteAddr, &addr_len);
	remoteAddr_p = (struct sockaddr_in *)&remoteAddr;
	strcpy(server3IPStr, inet_ntoa(remoteAddr_p->sin_addr));
	printf("The Server 2 received the value %s from the Server 3 with port number %s and IP address %s.\n", recvBuf, SERVER3_PORT, server3IPStr);

	close(TCPSockfd);
	printf("The Server 2 closed the TCP connection with Server 3.\n");

    char *str = recvBuf;
    return str;
}
//main function
int main(void){

	int sockfd, childSockfd;
	struct addrinfo localHost, *p;
	struct sockaddr remotAddr;
	
	socklen_t sin_size = sizeof remotAddr;
	char str[INET_ADDRSTRLEN];
	int rv;
// read file fomr txt file and then return current table size
   	int tableSize = readFile(table, "server2.txt");

	memset(&localHost, 0, sizeof localHost);
	localHost.ai_family = AF_INET;
	localHost.ai_socktype = SOCK_STREAM;
	localHost.ai_flags = AI_PASSIVE;
	
	if(getaddrinfo(NULL, PORT, &localHost, &p)!=0){
		printf("get address information in error!\n");
		return 0;
	}
//create a TCP socket	
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

//show TCP port number and IP address of local server2
		{
		struct sockaddr localAddr;
		int addr_len = sizeof localAddr;
		struct sockaddr_in *local_p;

		getsockname(sockfd, &localAddr, &addr_len);
		local_p = (struct sockaddr_in*)&localAddr;
	    struct hostent *h = gethostbyname("localhost");     
	    localIPStr = inet_ntoa(*((struct in_addr*)(h->h_addr)));
		printf("The Server 2 has TCP port number %s and IP address %s.\n",PORT, localIPStr); 
		}
//accept server1
		childSockfd = accept(sockfd, &remotAddr, &sin_size);	
		if(childSockfd==-1){
			perror("accept");
			return 0;
		}
//TCP receive from server1	
		if(recv(childSockfd, recvBuf, MAXBUFFLEN, 0)==-1){
			perror("recv\n");
			return 0;
		}
//search table
	    keyValue = malloc(10*sizeof(char));
	    removeCommand(keyValue, recvBuf);
	    matchValue = searchTable(table, keyValue, tableSize);
//show key value and port number&IP address of server1
		{
		struct sockaddr_in *remoteAddr_p;
		remoteAddr_p = (struct sockaddr_in *)&remotAddr;
	    clientTCPPortNum = (int)ntohs(remoteAddr_p->sin_port);
	    strcpy(clientTCPIPStr,  inet_ntoa(remoteAddr_p->sin_addr));
		printf("The Server 2 has received a request with key %s from the Server 1 with port number %d and IP address %s.\n", recvBuf, clientTCPPortNum,clientTCPIPStr);
		}
		
//whether found a matched value 	
		if(matchValue!=NULL){
			addCommand(sendBuf, matchValue, "POST");
			printf("The Server 2 sends the reply %s to the Server 1 with port number %d and IP address %s.\n", sendBuf, clientTCPPortNum,clientTCPIPStr); 
			if(send(childSockfd, sendBuf, strlen(sendBuf)+1, 0)==-1){
				perror("server2: send back in error\n");
				return 0;	
			}
		}
		else{
//cannot find a value
    		char recvFromServer3[MAXBUFFLEN];
    		strcpy(recvFromServer3, askNextServer(SERVER3_PORT, recvBuf));
		    if(send(childSockfd, recvFromServer3, strlen(recvFromServer3)+1, 0)==-1){
				perror("server2: send back in error\n");
				return 0;	
		    }
		    printf("The Server2, sent reply %s to Server 1 with port number %d and IP address %s.\n", recvFromServer3,clientTCPPortNum, clientTCPIPStr); 
		}

		close(childSockfd);
	}

	close(sockfd);
	return 0;
}
