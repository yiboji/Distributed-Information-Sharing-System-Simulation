#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>

#define UDP_SERVER 1
#define TRUE 1
#define FALSE 0
#define PORT "21887"
#define MAXBUFFLEN 100
#define SERVER2_PORT "22887"

int UDPSockfd;
char UDPRecvBuf[MAXBUFFLEN];
char UDPSendBuf[MAXBUFFLEN];

struct sockaddr clientAddr;
socklen_t addr_len = sizeof clientAddr;
struct sockaddr *clientAddr_p = &clientAddr;
char *localIPStr;
char remoteIPStr[INET_ADDRSTRLEN];
int  remotePortNum;

char remoteTCPIPStr[INET_ADDRSTRLEN];
int  localTCPPortNum;
int  remoteTCPPortNum;


char TCPRecvBuf[MAXBUFFLEN];

struct Table{
	char *key;
	char *value;
};
struct Table table[50];
//read from file and store in struct Table array
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
//initiate UDP
int UDPInitialization(){

	struct addrinfo hostAddr, *p;
	
	memset(&hostAddr, 0, sizeof hostAddr);
	hostAddr.ai_family = AF_INET;
	hostAddr.ai_socktype = SOCK_DGRAM;
	hostAddr.ai_flags = AI_PASSIVE;

	if(getaddrinfo(NULL, PORT, &hostAddr, &p)!=0){
		printf("get host address infomation in error\n");
		return 1;
	}
	
//create a UDP socket
	if((UDPSockfd=socket(p->ai_family,p->ai_socktype, p->ai_protocol))==-1){
		perror("listener: socket");
	}
//show UDP ip address and port number of server1
	{
		struct sockaddr localAddr;
		int addr_len = sizeof localAddr;
		struct sockaddr_in *local_p;
		getsockname(UDPSockfd, &localAddr, &addr_len);
		local_p = (struct sockaddr_in*)&localAddr;
        	struct hostent *h = gethostbyname("localhost");     
        	localIPStr = inet_ntoa(*((struct in_addr*)(h->h_addr)));
 
		printf("The Sever 1 has UDP port number %s and IP address %s\n", PORT, localIPStr);	 
	}
//bind UDP	
	if(bind(UDPSockfd, p->ai_addr, p->ai_addrlen)==-1){
		close(UDPSockfd);
		perror("listener: bind");
	}
	
	if(p==NULL){
		printf("fail to bind socket");
		return 2;
	}
	return 0;
}
//main function
int main(void){

	char *matchValue;
	char *keyValue;
	char content[MAXBUFFLEN];
	int findValue;
	char postCommand[MAXBUFFLEN];
	char getCommand[MAXBUFFLEN];

	int tableSize = readFile(table, "server1.txt");
	int clientFlag = 0;
//UDP initialization
	if(UDPInitialization())
		exit(1);

	while(1){
		clientFlag = ~clientFlag;
//UDP receiving
		if(recvfrom(UDPSockfd, UDPRecvBuf, MAXBUFFLEN-1, 0, &clientAddr, &addr_len)==-1){
			perror("recvfrom");
			exit(1);
		}
//search table
		removeCommand(content, UDPRecvBuf);
		keyValue = content;
		matchValue = searchTable(table, keyValue, tableSize);	
//show the key value and port number&IP address of client 1
		inet_ntop(AF_INET,&((struct sockaddr_in*)clientAddr_p)->sin_addr,remoteIPStr, sizeof remoteIPStr);
		remotePortNum = ((struct sockaddr_in*)clientAddr_p)->sin_port;	
		if(clientFlag)
			printf("The Server 1 has received a request with key %s from client 1 with port number %d and IP address %s\n", keyValue,remotePortNum, remoteIPStr);
		else
			printf("The Server 1 has received a request with key %s from client 2 with port number %d and IP address %s\n", keyValue,remotePortNum, remoteIPStr);
//whether a value was found
		if(matchValue!=NULL){
			addCommand(postCommand, matchValue, "POST");
//printf("server1: key value is \"%s\"\n", matchValue);	
			sendto(UDPSockfd, postCommand, strlen(postCommand)+1, 0, &clientAddr, addr_len);
			if(clientFlag)
				printf("The server 1 sends the reply %s to Client 1 with port number %d and IP address %s\n",postCommand,remotePortNum,remoteIPStr);
			else
				printf("The server 1 sends the reply %s to Client 2 with port number %d and IP address %s\n",postCommand,remotePortNum,remoteIPStr);
		}
		else{
//ask value for server2
			int TCPSockfd;
			struct addrinfo serverAddr, *p;
			int numBytes;
			memset(&serverAddr, 0, sizeof serverAddr);
			serverAddr.ai_family = AF_INET;
			serverAddr.ai_socktype = SOCK_STREAM;
//initiate TCP connection		
			if(getaddrinfo("localhost", SERVER2_PORT, &serverAddr, &p)==-1){
				printf("get address infomation in error!\n");
				return 0;	
			}
			TCPSockfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol);
			if(TCPSockfd==-1){
				perror("server1: client: socket:");
				return 0;
			}
			if(connect(TCPSockfd, p->ai_addr, p->ai_addrlen)==-1){
				perror("server1: TCP client: connect");
				return 0;
			}
//show TCP port number and IP address of server1
			struct sockaddr localAddr;
			int addr_len = sizeof localAddr;
			struct sockaddr_in *local_p;
			getsockname(TCPSockfd, &localAddr, &addr_len);
			local_p = (struct sockaddr_in*)&localAddr;
			localTCPPortNum = ntohs(local_p->sin_port); 
			printf("The Server 1 sends the request %s to the Server2.\n",  UDPRecvBuf);	 
			printf("The TCP port number is %d and the IP address is %s.\n", localTCPPortNum, localIPStr);
//Forword message to server2.
			if(send(TCPSockfd, UDPRecvBuf, strlen(UDPRecvBuf)+1, 0)==-1){
				perror("send");
				return 0;
			}
			if(recv(TCPSockfd, TCPRecvBuf, MAXBUFFLEN, 0)==-1){
				perror("server1: receive from server2 in error\n");
				return 0;
			} 
		
//show value and port number&IP address of server2
			{
			struct sockaddr remoteAddr;
			struct sockaddr_in *remoteAddr_p;
			int addr_len = sizeof remoteAddr;

			getpeername(TCPSockfd, &remoteAddr, &addr_len);
			remoteAddr_p = (struct sockaddr_in *)&remoteAddr;
			strcpy(remoteTCPIPStr, inet_ntoa(remoteAddr_p->sin_addr));
			remoteTCPPortNum = ntohs(remoteAddr_p->sin_port);
			printf("The Server 1 received the value %s from the Server 2 with port number %d and IP address %s.\n", TCPRecvBuf, remoteTCPPortNum, remoteTCPIPStr);

			}
			close(TCPSockfd);
			printf("The Server 1 closed the TCP connection with Server 2.\n");
//send value back to client via UDP
			sendto(UDPSockfd, TCPRecvBuf, strlen(TCPRecvBuf)+1, 0, &clientAddr, addr_len); 
			if(clientFlag)
				printf("The Server 1, sent reply %s to Client 1 with port number %d and IP address %s.\n", TCPRecvBuf, remotePortNum, remoteIPStr);
			else
				printf("The Server 1, sent reply %s to Client 2 with port number %d and IP address %s.\n", TCPRecvBuf, remotePortNum, remoteIPStr);
//store key and value in local table
		char recValue[MAXBUFFLEN];
		removeCommand(recValue, TCPRecvBuf);	
		table[tableSize].key = malloc(10*sizeof(char));
		table[tableSize].value = malloc(10*sizeof(char));
		strcpy(table[tableSize].key, keyValue);
		strcpy(table[tableSize].value, recValue);
		tableSize++;
		}
	}
	close(UDPSockfd);
	return 0;
}


