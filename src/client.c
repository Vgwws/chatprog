#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "def.h"

int sockfd;

void* get_message(void* args){
	(void)args;
	char buffer[BUFFER_SIZE];
	while(1){
		int recv_len = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
		if(recv_len <= 0){
			printf("Disconnected from server\n");
			break;
		}
		buffer[BUFFER_SIZE - 1] = '\0';
		printf("User: %s", buffer);
		fflush(stdout);
	}
	exit(0);
	return NULL;
}

int main(void){
	struct sockaddr_in server_addr;
	pthread_t tid;
	char buffer[BUFFER_SIZE];
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		fprintf(stderr, "ERROR: Socket failed\n");
		return 1;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(connect(sockfd, 
				(struct sockaddr*)&server_addr, 
				sizeof(struct sockaddr_in)) < 0){
		fprintf(stderr, "ERROR: Can't connect to server\n");
		return 1;
	}
	printf("Connected to server\n");
	if(pthread_create(&tid, NULL, get_message, NULL) != 0){
		fprintf(stderr, "ERROR: Can't create thread\n");
		return 1;
	}
	while(1){
		if(!fgets(buffer, BUFFER_SIZE, stdin))
			break;
		if(send(sockfd, buffer, strlen(buffer), 0) < 0)
			fprintf(stderr, "ERROR: Can't send message to server\n");
	}
	close(sockfd);
	return 0;
}
