SRCMODULES = ../lib/tty/tty.c

.PHONY: build
build: clean unbuffer longrunner

-include ../include/main.mk

%: %.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean_%
clean_%:
	rm -f $*.o $*

.PHONY: clean
clean: clean_unbuffer clean_longrunner;
