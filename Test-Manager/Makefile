C = cc
CSTD = -std=c99
CFLAGS = -Wall -Wextra -Werror -pedantic

SRCS = $(wildcard *.c)
TARGETS = $(SRCS:.c=)

all: server.o
	$(C) $(CSTD) $(CFLAGS) -o server server.o -lm

server.o : server.c server.h
	$(C) $(CSTD) $(CFLAGS)  -c server.c

clean:
	rm -f $(TARGETS)