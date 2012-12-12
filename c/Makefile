#===============================================================
# 'make -p' to checkout default variables and rules
#===============================================================
CFLAGS = -g -pg -Wall -O3 --std=c99 `pkg-config --cflags gsl` -DTEST_MYOP
LDFLAGS = -lm -lc `pkg-config --libs gsl`
LDLIBS =
CC = gcc
PROGS = erf gsl_erf rand lib
TRASHES = *.out *.dSYM

all: $(PROGS)

erf: erf.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ erf.c

rand: rand.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ rand.c

gsl_erf: gsl_erf.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ gsl_erf.c

lib: lib.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ lib.c

clean:
	$(RM) $(PROGS)
	$(RM) -r $(TRASHES)
