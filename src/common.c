#include <string.h>
#include <arpa/inet.h>

#include "common.h"

u8 send_str(int sockfd, const char* str){
	u32 raw_len = htonl(strlen(str));
	if(send(sockfd, &raw_len, 4, 0) < 0)
		return 1;
	if(send(sockfd, str, strlen(str), 0) < 0)
		return 1;
	return 0;
}

int accept_str(int sockfd, char* str){
	u32 raw_len;
	int recv_len = recv(
			sockfd, &raw_len, 4, 0);
	if(recv_len <= 0)
		return recv_len;
	u32 len = ntohl(raw_len);
	recv_len = recv(
			sockfd, str, len, 0);
	str[len] = '\0';
	return recv_len;
}
