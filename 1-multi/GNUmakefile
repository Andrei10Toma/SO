CC = gcc
CFLAGS = -Wall -Wextra -g

build: so-cpp

so-cpp: so-cpp.o hash_table.o vector.o utils.o
	$(CC) $(CFLAGS) $^ -o $@

hash_table.o: hash_table.c hash_table.h
vector.o: vector.c vector.h
so-cpp.o: so-cpp.c
utils.o: utils.c utils.h

clean:
	rm so-cpp *.o
