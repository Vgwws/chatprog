.PHONY: all clean client_run server_run

NAME := chatprog

CFLAGS := -c -I ../include -Wall -Wextra -Werror -std=gnu99

SERVER_OBJ := server.o \
							env_parser.o \
							db.o \
							common.o

CLIENT_OBJ := client.o \
							common.o

all: $(NAME)-server $(NAME)

%.o: %.c
	gcc $(CFLAGS) -O2 $< -o $@

$(NAME)-server: $(SERVER_OBJ)
	gcc $^ -o $@ -lpq

$(NAME): $(CLIENT_OBJ)
	gcc $^ -o $@

clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) server client

server_run: $(NAME)-server
	./server

client_run: $(NAME)
	./client
