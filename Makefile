CC=gcc
CFLAGS=-Wall -O0 -g

#test: run Makefile int.c
#	$(CC) $(CFLAGS) --analyze int.c 

run: int.c
	$(CC) $(CFLAGS) int.c -o run
