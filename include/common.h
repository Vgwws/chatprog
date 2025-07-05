#ifndef COMMON_H
#define COMMON_H

#define PORT 9001
#define BUFFER_SIZE 1024

typedef __UINT64_TYPE__ u64;
typedef __UINT32_TYPE__ u32;
typedef __UINT16_TYPE__ u16;
typedef __UINT8_TYPE__ u8;

u8 send_str(int sockfd, const char* str);
int accept_str(int sockfd, char* str);

#endif
