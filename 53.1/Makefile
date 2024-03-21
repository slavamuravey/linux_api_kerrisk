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
