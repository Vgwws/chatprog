#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "common.h"

int sockfd;

u8 handle_login(void){
	while(1){
		char username[BUFFER_SIZE];
		char code[4];
		char response[BUFFER_SIZE];
		printf("Username: ");
		if(!fgets(username, sizeof(username), stdin)){
			fprintf(stderr, "ERROR: Can't read username\n");
			return 1;
		}
		username[strlen(username) - 1] = '\0';
		if(send_str(sockfd, username)){
			fprintf(stderr, "ERROR: Can't send username info to server\n");
			return 1;
		}
		printf("Waiting for response...\n");
		if(accept_str(sockfd, code) <= 0){
			printf("Disconnected from server\n");
			return 1;
		}
		if(accept_str(sockfd, response) <= 0){
			printf("Disconnected from server\n");
			return 1;
		}
		printf("%s\n", code);
		if(!strcmp(code, "BAD")){
			fprintf(stderr, "%s", response);
			continue;
		}
		char password[BUFFER_SIZE];
		printf("Password: ");
		if(!fgets(password, sizeof(password), stdin)){
			fprintf(stderr, "ERROR: Can't read password\n");
			return 1;
		}
		password[strlen(password) - 1] = '\0';
		if(send_str(sockfd, password)){
			fprintf(stderr, "ERROR: Can't send password info to server\n");
			return 1;
		}
		printf("Waiting for response...\n");
		if(accept_str(sockfd, code) <= 0){
			printf("Disconnected from server\n");
			return 1;
		}
		if(accept_str(sockfd, response) <= 0){
			printf("Disconnected from server\n");
			return 1;
		}
		printf("%s\n", code);
		if(!strcmp(code, "BAD")){
			fprintf(stderr, "%s", response);
			continue;
		}
		break;
	}
	return 0;
}

void* get_message(void* args){
	(void)args;
	while(1){
		char username[BUFFER_SIZE];
		char message[BUFFER_SIZE];
		u32 username_len_raw;
		int recv_len = recv(sockfd, &username_len_raw, 4, 0);
		if(recv_len <= 0){
			printf("Disconnected from server\n");
			break;
		}
		u32 username_len = ntohl(username_len_raw);
		recv_len = recv(sockfd, username, username_len, 0);
		if(recv_len <= 0){
			printf("Disconnected from server\n");
			break;
		}
		u32 message_len_raw;
		recv_len = recv(sockfd, &message_len_raw, 4, 0);
		if(recv_len <= 0){
			printf("Disconnected from server\n");
			break;
		}
		u32 message_len = ntohl(message_len_raw);
		recv_len = recv(sockfd, message, message_len, 0);
		if(recv_len <= 0){
			printf("Disconnected from server\n");
			break;
		}
		printf("[%s]: %s", username, message);
		fflush(stdout);
	}
	exit(0);
	return NULL;
}

int main(void){	
	struct sockaddr_in server_addr;
	pthread_t tid;
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
	if(handle_login())
		return 1;
	if(pthread_create(&tid, NULL, get_message, NULL) != 0){
		fprintf(stderr, "ERROR: Can't create thread\n");
		return 1;
	}
	while(1){
		char buffer[BUFFER_SIZE];
		if(!fgets(buffer, sizeof(buffer), stdin))
			break;
		printf("You: %s", buffer);
		if(send_str(sockfd, buffer))
			fprintf(stderr, "ERROR: Can't send to server\n");
	}
	close(sockfd);
	return 0;
}
