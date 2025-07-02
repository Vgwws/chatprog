.PHONY: all clean client_run server_run

CFLAGS := -c -I ../include -Wall -Wextra -Werror -std=gnu99

SERVER_OBJ := server.o \
							env_parser.o \
							db.o

CLIENT_OBJ := client.o

all: server client

%.o: %.c
	gcc $(CFLAGS) -O2 $< -o $@

server: $(SERVER_OBJ)
	gcc $^ -o $@ -lpq

client: $(CLIENT_OBJ)
	gcc $^ -o $@

clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) server client

server_run: server
	./server

client_run: client
	./client
