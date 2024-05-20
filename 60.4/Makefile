SRCMODULES = ../lib/tty/tty.c

.PHONY: build
build: clean script script_replay

-include ../include/main.mk

%: %.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean_%
clean_%:
	rm -f $*.o $*

.PHONY: clean
clean: clean_script clean_script_replay;
