CC=clang
CFLAGS=-Wall -O0 -g

test: run Makefile huffman.c
	$(CC) $(CFLAGS) --analyze huffman.c 

run: huffman.c
	$(CC) $(CFLAGS) huffman.c -o huffman
