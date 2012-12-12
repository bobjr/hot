#===============================================================
# 'make -p' to checkout default variables and rules
#===============================================================
CFLAGS = -g -Wall -O3 --std=c99 `pkg-config --cflags gsl`
LDFLAGS = -lm -lc `pkg-config --libs gsl`
LDLIBS =
CC = gcc
PROGS = erf gsl_erf
TRASHES = a.out erf.dSYM gsl_erf.dSYM

all: $(PROGS)

erf: erf.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ erf.c

gsl_erf: gsl_erf.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ gsl_erf.c


clean:
	$(RM) $(PROGS)
	$(RM) -r $(TRASHES)
