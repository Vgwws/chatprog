#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "db.h"

#define MAX_CLIENTS 2

int clients_socket[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void err_exit(const char* msg, unsigned int index, void* args){
	fprintf(stderr, "ERROR: %s\n", msg);
	close(clients_socket[index]);
	clients_socket[index] = -1;
	free(args);
}

u8 send_code(const char* code, const char* msg, unsigned int index, void* args){
	printf("Code: %s\n", code);
	printf("Message: %s\n", msg);
	if(send(clients_socket[index], code, strlen(code), 0) < 0){
		err_exit("Can't send code to client", index, args);
		return 1;
	}
	if(send(clients_socket[index], msg, strlen(msg), 0) < 0){
		err_exit("Can't send message to client", index, args);
		return 1;
	}
	return 0;
}

void* client_handle(void* args){
	char username[BUFFER_SIZE];
	unsigned int index = *(unsigned int*)args;
	int username_size;
	while(1){
		printf("Get Username and Password\n");
		char password[BUFFER_SIZE];
		username_size = recv(
				clients_socket[index], username, BUFFER_SIZE, 0);
		if(username_size == 0){
			printf("Client disconnected\n");
			close(clients_socket[index]);
			clients_socket[index] = -1;
			free(args);
			pthread_exit(NULL);
		}
		if(username_size < 0){
			err_exit("Recv failed", index, args);
			pthread_exit(NULL);
		}
		printf("Username: %s\n", username);
		if(!strcmp(username, "OK") || 
				!strcmp(username, "BAD")){
			if(send_code("BAD", "Can't use system name\n", index, args))
				pthread_exit(NULL);	
			continue;
		}
		if(send_code("OK", "\n", index, args))
			pthread_exit(NULL);
		int password_size = recv(
				clients_socket[index], password, BUFFER_SIZE, 0);
		if(password_size == 0){
			printf("Client disconnected\n");
			close(clients_socket[index]);
			clients_socket[index] = -1;
			free(args);
			pthread_exit(NULL);
		}
		if(password_size < 0){
			err_exit("Recv failed", index, args);
			pthread_exit(NULL);
		}
		u8 error = db_user_login(username, password);
		switch(error){
			case 2:
				if(send_code("BAD", "Invalid Password\n", index, args))
					pthread_exit(NULL);
				continue;
			case 1:
				db_exit();
				exit(1);
			default:
				break;
		}
		break;
	}
	while(1){
		printf("Handle messaging\n");
		char message[BUFFER_SIZE];
		int message_size = recv(
				clients_socket[index], message, BUFFER_SIZE, 0);
		if(message_size == 0){
			printf("Client disconnected\n");
			break;
		}
		if(message_size < 0){
			fprintf(stderr, "ERROR: Recv failed\n");
			break;
		}
		if(db_insert_new_message(username, message)){
			db_exit();
			exit(1);
		}
		pthread_mutex_lock(&clients_mutex);
		for(unsigned int i = 0; i < MAX_CLIENTS; i++){
			if(i == index || clients_socket[i] < 0)
				continue;
			if(send(clients_socket[i], username, username_size, 0) < 0){
				fprintf(stderr, "ERROR: Can't send to client\n");
				continue;
			}
			if(send(clients_socket[i], message, message_size, 0) < 0){
				fprintf(stderr, "ERROR: Can't send to client\n");
				continue;
			}
		}
		pthread_mutex_unlock(&clients_mutex);
	}
	close(clients_socket[index]);
	clients_socket[index] = -1;
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
	db_init();
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
	db_exit();
	return 0;
}
