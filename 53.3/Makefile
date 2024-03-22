SRCMODULES = ../lib/error_functions.c ../lib/get_num.c

.PHONY: build
build: clean client server

-include ../include/main.mk

%: us_seqnum_%.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o us_seqnum_$@

.PHONY: clean_%
clean_%:
	rm -f us_seqnum_$*.o us_seqnum_$*

.PHONY: clean
clean: clean_client clean_server;
