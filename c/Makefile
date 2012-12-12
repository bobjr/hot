# comment 
CFLAGS = -g -Wall -O3 --std=c99
LDLIBS =
CC = c99
PROGS = erf

all: $(PROGS)

erf:
	gcc -o erf erf.c -lm -lc -g -Wall -O3 --std=c99

clean:
	rm -f a.out
	rm -f erf
	rm -rf erf.dSYM
