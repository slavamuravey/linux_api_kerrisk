SRCMODULES = ../lib/error_functions.c ../lib/get_num.c ../lib/socket/inet_sockets.c ../lib/become_daemon.c ../lib/tty/tty.c

.PHONY: build
build: clean client server

-include ../include/main.mk

%: %.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean_%
clean_%:
	rm -f $*.o $*

.PHONY: clean
clean: clean_client clean_server;
