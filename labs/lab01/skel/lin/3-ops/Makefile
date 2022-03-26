CC = gcc
CFLAGS = -Wall -DHAVE_MATH
LDLIBS = -lm

all: ops

ops: ops.o mul.o add.o
	gcc $(CFLAGS) $^ -o $@ $(LDLIBS)

mul.o: mul.c
add.o: add.c
ops.o: ops.c

clean:
	rm -rf *.o ops

