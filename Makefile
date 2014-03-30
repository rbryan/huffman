CC=gcc
CFLAGS=-Wall -O3 -g

#test: run Makefile int.c
#	$(CC) $(CFLAGS) --analyze int.c 

run: int.c
	$(CC) $(CFLAGS) int.c -o run
