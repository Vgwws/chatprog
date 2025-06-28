#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "def.h"

#define MAX_CLIENTS 2

int clients_socket[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void* client_handle(void* args){
	char message[BUFFER_SIZE];
	char username[BUFFER_SIZE];
	unsigned int index_socket = *(unsigned int*)args;
	int username_size = recv(
			clients_socket[index_socket], username, BUFFER_SIZE, 0);
	if(username_size == 0){
		printf("Client disconnected\n");
		close(clients_socket[index_socket]);
		clients_socket[index_socket] = -1;
		free(args);
		pthread_exit(NULL);
	}
	if(username_size < 0){
		fprintf(stderr, "ERROR: Recv failed\n");
		close(clients_socket[index_socket]);
		clients_socket[index_socket] = -1;
		free(args);
		pthread_exit(NULL);
	}
	while(1){
		int message_size = recv(
				clients_socket[index_socket], message, BUFFER_SIZE, 0);
		if(message_size == 0){
			printf("Client disconnected\n");
			break;
		}
		if(message_size < 0){
			fprintf(stderr, "ERROR: Recv failed\n");
			break;
		}
		pthread_mutex_lock(&clients_mutex);
		for(unsigned int i = 0; i < MAX_CLIENTS; i++){
			if(i == index_socket || clients_socket[i] < 0)
				continue;
			if(send(clients_socket[i], username, username_size, 0) < 0){
				fprintf(stderr, "ERROR: Send failed\n");
				continue;
			}
			if(send(clients_socket[i], message, message_size, 0) < 0){
				fprintf(stderr, "ERROR: Send failed\n");
				continue;
			}
		}
		pthread_mutex_unlock(&clients_mutex);
	}
	close(clients_socket[index_socket]);
	clients_socket[index_socket] = -1;
	free(args);
	pthread_exit(NULL);
}

int main(void){
	int server_socket;
	struct sockaddr_in server_addr, client_addr[MAX_CLIENTS];
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket < 0){
		fprintf(stderr, "ERROR: Socket failed\n");
		return 1;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);
	if(bind(server_socket, 
				(struct sockaddr*)&server_addr, 
				sizeof(struct sockaddr_in)) < 0){
		fprintf(stderr, "ERROR: Bind failed\n");
		return 1;
	}
	if(listen(server_socket, 5) < 0){
		fprintf(stderr, "ERROR: Listen failed\n");
		return 1;
	}
	pthread_t tid[MAX_CLIENTS];
	socklen_t client_len = sizeof(struct sockaddr_in);
	for(unsigned int i = 0; i < MAX_CLIENTS; i++){
		clients_socket[i] = -1;
	}
	while(1){
		int index = -1;
		for(unsigned int i = 0; i < MAX_CLIENTS; i++){
			if(clients_socket[i] < 0){
				index = i;
				break;
			}
		}
		if(index < 0){
			fprintf(stderr, "ERROR: Can't accept more clients\n");
			int temp_sock = accept(server_socket, NULL, NULL);
			close(temp_sock);
			continue;
		}
		clients_socket[index] = accept(
				server_socket, (struct sockaddr*)&client_addr[index], &client_len);
		if(clients_socket[index] < 0){
			fprintf(stderr, "ERROR: Can't accept client\n");
			continue;
		}
		unsigned int* pindex = malloc(sizeof(unsigned int));
		if(!pindex){
			fprintf(stderr, "ERROR: Can't allocate memory for pindex\n");
			close(clients_socket[index]);
			clients_socket[index] = -1;
			continue;
		}
		*pindex = index;
		if(pthread_create(&tid[index], NULL, client_handle, pindex) != 0){
			fprintf(stderr, "ERROR: Can't create thread\n");
			close(clients_socket[index]);
			clients_socket[index] = -1;
			free(pindex);
		}
	}
	return 0;
}
