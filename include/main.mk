CC = gcc
CFLAGS = -g -Wall -ansi -pedantic
SRCMODULES ?= 
OBJMODULES = $(SRCMODULES:.c=.o)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: run
run: clean main
	./main

main: main.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f *.o main
