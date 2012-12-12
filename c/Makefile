# comment 
CFLAGS = -g -Wall -O3 --std=c99
LDFLAGS = -lm -lc
LDLIBS =
CC = gcc
PROGS = erf

all: $(PROGS)

erf: erf.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ erf.c

clean:
	$(RM) $(PROGS)
	$(RM) a.out
	$(RM) -r erf.dSYM
