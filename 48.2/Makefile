SRCMODULES = ../lib/error_functions.c ../lib/get_num.c

.PHONY: build
build: clean client server

-include ../include/main.mk

%: pmsg_seqnum_%.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o pmsg_$@

.PHONY: clean_%
clean_%:
	rm -f pmsg_seqnum_$*.o pmsg_seqnum_$*

.PHONY: clean
clean: client server;
