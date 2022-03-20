CC = gcc
CFLAGS = -Wall -Wextra -g

build: so-cpp

so-cpp: so-cpp.o hash_table.o vector.o utils.o
	$(CC) $(CFLAGS) $^ -o $@

hash_table.o: hash_table.c
vector.o: vector.c
so-cpp.o: so-cpp.c
utils.o: utils.c

clean:
	rm so-cpp *.o
