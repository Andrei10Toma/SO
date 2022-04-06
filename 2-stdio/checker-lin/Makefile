CC = gcc
CFLAGS = -Wall -Wextra -g

build: libso_stdio.so

libso_stdio.so: so_stdio.o 
	$(CC) -shared $^ -o $@ -L. $(CFLAGS)

so_stdio.o: so_stdio.c so_stdio.h
	$(CC) -fPIC -c $< -o $@ $(CFLAGS)

clean:
	rm *.o libso_stdio.so
