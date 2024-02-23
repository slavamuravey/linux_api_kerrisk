SRCMODULES = ../lib/error_functions.c ../lib/get_num.c

.PHONY: build
build: clean server client

-include ../include/main.mk

server: fifo_seqnum_server.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@

client: fifo_seqnum_client.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean_server
clean_server:
	rm -f server.o server

.PHONY: clean_client
clean_client:
	rm -f client.o client

.PHONY: clean
clean: clean_server clean_client;
