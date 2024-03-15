SRCMODULES = ../lib/error_functions.c ../lib/get_num.c

.PHONY: build
build: clean create unlink getattr receive timedreceive send

-include ../include/main.mk

%: pmsg_%.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o pmsg_$@

.PHONY: clean_%
clean_%:
	rm -f pmsg_$*.o pmsg_$*

.PHONY: clean
clean: clean_create clean_unlink clean_getattr clean_receive clean_timedreceive clean_send;
