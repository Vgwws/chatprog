#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "def.h"

int sockfd;

u8 handle_prompt(char* prompt){
	char response_name[BUFFER_SIZE];
	char response_msg[BUFFER_SIZE];
	while(1){
		printf("Username\n");
		char input[BUFFER_SIZE];
		printf("%s: ", prompt);
		if(!fgets(input, BUFFER_SIZE, stdin)){
			fprintf(stderr, "ERROR: Can't read %s\n", prompt);
			return 1;
		}
		input[strlen(input) - 1] = '\0';
		if(send(sockfd, input, strlen(input), 0) < 0){
			fprintf(stderr, "ERROR: Can't send %s info to server\n", prompt);
			return 1;
		}
		int name_len = recv(sockfd, response_name, BUFFER_SIZE - 1, 0);
		int message_len = recv(sockfd, response_msg, BUFFER_SIZE - 1, 0);
		if(name_len <= 0 || message_len <= 0){
			printf("Disconnected from server\n");
			return 1;
		}
		printf("%s\n", response_name);
		if(!strcmp(response_name, "OK"))
			break;
		printf("%s", response_msg);
	}
	return 0;
}

void* get_message(void* args){
	(void)args;
	char buffer[BUFFER_SIZE];
	char input[BUFFER_SIZE];
	while(1){
		int input_len = recv(sockfd, input, BUFFER_SIZE - 1, 0);
		int message_len = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
		if(message_len <= 0 || input_len <= 0){
			printf("Disconnected from server\n");
			break;
		}	
		printf("[%s]: %s", input, buffer);
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
	if(handle_prompt("Username"))
		return 1;
	if(handle_prompt("Password"))
		return 1;
	if(pthread_create(&tid, NULL, get_message, NULL) != 0){
		fprintf(stderr, "ERROR: Can't create thread\n");
		return 1;
	}
	while(1){
		if(!fgets(buffer, BUFFER_SIZE, stdin))
			break;
		if(send(sockfd, buffer, strlen(buffer), 0) < 0){
			fprintf(stderr, "ERROR: Can't send message to server\n");
			break;
		}
	}
	close(sockfd);
	return 0;
}
