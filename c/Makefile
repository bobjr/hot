#===============================================================
# 'make -p' to checkout default variables and rules
#===============================================================
CFLAGS = -g -Wall -O3 --std=c99 `pkg-config --cflags sqlite3`
LDFLAGS = -lm -lc `pkg-config --libs sqlite3`
LDLIBS =
CC = gcc
PROGS = erf
TRASHES = a.out erf.dSYM

all: $(PROGS)

erf: erf.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ erf.c

clean:
	$(RM) $(PROGS)
	$(RM) -r $(TRASHES)
