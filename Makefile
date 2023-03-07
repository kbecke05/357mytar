CC = gcc
CFLAGS = -ansi -Wall -pedantic -g

mytar: mytar.c
	$(CC) $(CFLAGS) mytar.c -o mytar