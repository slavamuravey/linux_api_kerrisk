SRCMODULES = ../lib/error_functions.c ../lib/get_num.c ../lib/socket/inet_sockets.c ../lib/become_daemon.c

.PHONY: build
build: clean cl sv

-include ../include/main.mk

%: is_echo_%.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o is_echo_$@

.PHONY: clean_%
clean_%:
	rm -f is_echo_$*.o is_echo_$*

.PHONY: clean
clean: clean_cl clean_sv;
