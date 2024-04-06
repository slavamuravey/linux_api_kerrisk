SRCMODULES = ../lib/error_functions.c ../lib/get_num.c ../lib/socket/read_line.c

.PHONY: build
build: clean cl sv

-include ../include/main.mk

%: is_seqnum_%.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o is_seqnum_$@

.PHONY: clean_%
clean_%:
	rm -f is_seqnum_$*.o is_seqnum_$*

.PHONY: clean
clean: clean_cl clean_sv;
