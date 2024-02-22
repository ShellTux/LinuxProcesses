.PHONY: clean, mrproper
CC = gcc
CFLAGS = -g -Wall -Wextra

all: main

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main: main.o
	$(CC) $(CFLAGS) -o $@ $+

clean:
	rm -f *.o

mrproper: clean
	rm -f main
