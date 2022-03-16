CC = gcc
CFLAGS = -Wall -Wextra -g

all: so-cpp

so-cpp: so-cpp.o hash_table.o
	$(CC) $(CFLAGS) $^ -o $@

hash_table.o: hash_table.c
so-cpp.o: so-cpp.c

clean:
	rm so-cpp *.o