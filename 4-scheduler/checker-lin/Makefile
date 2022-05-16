CC = gcc
CFLAGS = -Wall -g
LIBS = -pthread

build: libscheduler.so

libscheduler.so: so_scheduler.o priority_queue.o
	$(CC) -shared $^ -o $@ -L. $(CFLAGS)

so_scheduler.o: so_scheduler.c so_scheduler.h thread.h
	$(CC) -fPIC -c $< -o $@ $(CFLAGS)

priority_queue.o: priority_queue.c priority_queue.h thread.h
	$(CC) -fPIC -c $< -o $@ $(CFLAGS)

clean:
	rm *.o libscheduler.so