C = cc
CSTD = -std=c99
CFLAGS = -Wall -Wextra -Werror -pedantic

SRCS = $(wildcard *.c)
TARGETS = $(SRCS:.c=)

all: server.o QBclient.o
	$(C) $(CSTD) $(CFLAGS) -o server server.o -lm
	$(C) $(CSTD) $(CFLAGS) -o QBclient QBclient.o -lm

QBclient.o : QBclient.c QBclient.h
	$(C) $(CSTD) $(CFLAGS)  -c QBclient.c

server.o : server.c server.h
	$(C) $(CSTD) $(CFLAGS)  -c server.c

clean:
	rm -f $(TARGETS) *.o
	
clearn:
	rm -f $(TARGETS) *.o && clear
	