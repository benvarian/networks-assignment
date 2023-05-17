C = cc
CSTD = -std=c11
CFLAGS = -Wall -Wextra -Werror -pedantic

SRCS = $(wildcard *.c)
TARGETS = $(SRCS:.c=)

all: server.o BinarySearchTree.o Dictionary.o Entry.o LinkedList.o Node.o Queue.o fileio.o
	$(C) $(CSTD) $(CFLAGS) -o server server.o BinarySearchTree.o Dictionary.o Entry.o LinkedList.o Node.o Queue.o fileio.o -lm

server.o : server.c server.h
	$(C) $(CSTD) $(CFLAGS)  -c server.c

fileio.o : ./fileio/fileio.c ./fileio/fileio.h
	$(C) $(CSTD) $(CFLAGS)  -c fileio/fileio.c

BinarySearchTree.o : ./Data-Structures/BinarySearchTree/BinarySearchTree.c ./Data-Structures/BinarySearchTree/BinarySearchTree.h
	$(C) $(CSTD) $(CFLAGS)  -c ./Data-Structures/BinarySearchTree/BinarySearchTree.c

Dictionary.o : ./Data-Structures/Dictionary/Dictionary.c ./Data-Structures/Dictionary/Dictionary.h
	$(C) $(CSTD) $(CFLAGS)  -c ./Data-Structures/Dictionary/Dictionary.c

Entry.o : ./Data-Structures/Entry/Entry.c ./Data-Structures/Entry/Entry.h
	$(C) $(CSTD) $(CFLAGS)  -c ./Data-Structures/Entry/Entry.c

LinkedList.o : ./Data-Structures/LinkedList/LinkedList.c ./Data-Structures/LinkedList/LinkedList.c
	$(C) $(CSTD) $(CFLAGS)  -c ./Data-Structures/LinkedList/LinkedList.c

Node.o : ./Data-Structures/Node/Node.c ./Data-Structures/Node/Node.h
	$(C) $(CSTD) $(CFLAGS)  -c ./Data-Structures/Node/Node.c

Queue.o : ./Data-Structures/Queue/Queue.c ./Data-Structures/Queue/Queue.h
	$(C) $(CSTD) $(CFLAGS)  -c ./Data-Structures/Queue/Queue.c

clean:
	rm -f $(TARGETS) *.o
	
clearn:
	rm -f $(TARGETS) *.o && clear
	