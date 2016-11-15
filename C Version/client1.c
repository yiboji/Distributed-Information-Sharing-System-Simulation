#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define SERVER_PORT "21887"
#define MAXBUFFLEN 100
#define UDP_CLIENT 0
#define UDP_SERVER 1

char *matchStr;
char *getCommand;
char *searchStr;
char recValue[MAXBUFFLEN];
char sendBuf[MAXBUFFLEN];

char remoteIPStr[INET_ADDRSTRLEN];
int localPortNum;

struct Table{
	char *key;
	char *value;
};
struct Table table[50];

//read txt file into struct Table
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
//search value by key from table and return and return the value
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
//remove "GET" or "POST" from received message
void removeCommand(char *dest, char *src){
	strtok(src, " ");
	strcpy(dest, strtok(NULL, "\0"));	
}
//add "GET" or "POST" on raw message
void addCommand(char *dest, char *src, char *command){
	strcpy(dest, command);
	strcat(dest, " ");
	strcat(dest, src);
}
//initiate UDP
int  initUDP(struct addrinfo *p_config, struct addrinfo **p_out, char *portNum, int server){
	int rv;
	int sockfd;
	struct addrinfo *p;
	char *addr;
	memset(p_config, 0, sizeof *p_config);
	p_config->ai_family = AF_INET;
	p_config->ai_socktype = SOCK_DGRAM;

	if(server==1){
		p_config->ai_flags = AI_PASSIVE;
		addr = NULL;
	}
	else
		addr = "localhost";

	rv = getaddrinfo(addr, portNum, p_config, p_out);
	if(rv!=0){
		printf("get host address information in error");
		return -1;
	}
	p = *p_out;
	//create a UDP socket
	sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol);
	if(sockfd==-1){
		perror("taler: socket");
	}
	if(server==1){
		bind(sockfd, p->ai_addr, p->ai_addrlen);
	}
	if(p==NULL){
		printf("client: failed to bind socket!");
	}
	return sockfd;
}
//main function
int main(void){

	int sockfd;
	struct addrinfo hostAddr,  localAddr,*p;
	struct sockaddr serverAddr;
	char *localIPStr;
	
//initiate table and read user input 
    searchStr = malloc(10*sizeof(char));
	printf("Please Enter Your Search(USC, UCLA etc.): ");
	scanf("%s", searchStr);
//search table for key 	
	matchStr = searchTable(table, searchStr, readFile(table,"client1.txt"));
	if(matchStr!=NULL)
		printf("The Client 1 has received a request with search word %s, which maps to key %s.\n", searchStr, matchStr);
//inital UDP
	sockfd = initUDP(&hostAddr, &p, SERVER_PORT, UDP_CLIENT);	
	if(sockfd==-1){
		perror("taler: socket");
		exit(1);
	}	
//convert to GET Command
	addCommand(sendBuf, matchStr,"GET");
//ask server1
	if(sendto(sockfd, sendBuf, strlen(sendBuf)+1, 0, p->ai_addr, p->ai_addrlen)==-1){
		perror("client: sendto");
		exit(1);
	}
//show ipadress and port number of server1
	{
		inet_ntop(p->ai_family, &((struct sockaddr_in *)p->ai_addr)->sin_addr, remoteIPStr, sizeof remoteIPStr);
		printf("The Client 1 send the request %s to the Server 1 with port number %s and IP address %s.\n",
				sendBuf, SERVER_PORT, remoteIPStr); 
	}
//get ip address and port number of local host
	{
       
		struct sockaddr localAddr;
		int addr_len = sizeof localAddr;
		struct sockaddr_in *local_p;
		getsockname(sockfd, &localAddr, &addr_len);
		local_p = (struct sockaddr_in*)&localAddr;
		localPortNum = ntohs(local_p->sin_port);
        struct hostent *h = gethostbyname("localhost");     
        localIPStr = inet_ntoa(*((struct in_addr*)(h->h_addr)));
       	printf("The Client1's port number is %d and the IP address is %s.\n", localPortNum, localIPStr);	 
	}
//start to receive from server1
	{
		char recvBuf[MAXBUFFLEN];
		int recvlen;
		struct sockaddr_in remaddr;
		socklen_t addrlen = sizeof remaddr;
		recvlen = recvfrom(sockfd, recvBuf, MAXBUFFLEN, 0, (struct sockaddr*)&remaddr, &addrlen);
	//remove "POST"
		removeCommand(recValue, recvBuf);
		printf("The Client1 received the value %s from the server1 with port number %s and IP address %s.\n",
				recValue, SERVER_PORT, remoteIPStr);
		printf("the client1's port number is %d and IP address is %s.\n", localPortNum, localIPStr); 
	}

	close(sockfd);
	
	return 0;
}
