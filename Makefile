.PHONY: all clean client_run server_run

all:
	$(MAKE) -C src all

clean:
	$(MAKE) -C src clean

client_run:
	$(MAKE) -C src client_run

server_run:
	$(MAKE) -C src server_run
