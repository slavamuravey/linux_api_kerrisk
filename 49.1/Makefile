SRCMODULES = ../lib/error_functions.c ../lib/get_num.c

.PHONY: build
build: clean create unlink getvalue post wait trywait timedwait

-include ../include/main.mk

%: psem_%.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o psem_$@

.PHONY: clean_%
clean_%:
	rm -f psem_$*.o psem_$*

.PHONY: clean
clean: clean_create clean_unlink clean_getvalue clean_post clean_wait clean_trywait clean_timedwait;
