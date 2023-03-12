CC = gcc
CFLAGS = -Wall -pedantic -g

all: mytar

mytar: mytar.o create.o util.o
	$(CC) -o mytar $(CFLAGS) mytar.o create.o util.o

mytar.o: mytar.c util.o
	$(CC) $(CFLAGS) -c mytar.c

create.o: create.c create.h util.o
	$(CC) $(CFLAGS) -c create.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

clean: mytar
	rm -f *.o *~

test: mytar
	~pn-cs357/demos/tryAsgn4