C = cc
CSTD = -std=c99
CFLAGS = -Wall -Wextra -Werror -pedantic

SRCS = $(wildcard *.c)
TARGETS = $(SRCS:.c=)


fileio : fileio.o
	$(C) $(CSTD) $(CFLAGS) -o fileio fileio.o -lm

fileio.o : fileio.c fileio.h
	$(C) $(CSTD) $(CFLAGS) -c fileio.c

clean:
	rm -f $(TARGETS) *.o