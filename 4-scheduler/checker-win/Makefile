CC = cl

build: libscheduler.dll

libscheduler.dll: so_scheduler.obj priority_queue.obj
	link /dll /out:$@ $**

priority_queue.obj: thread.h priority_queue.c priority_queue.h
	$(CC) /Fo$@ /c priority_queue.c
so_scheduler.obj: so_scheduler.c thread.h
	$(CC) /Fo$@ /c so_scheduler.c

clean:
	del *.obj so_scheduler.dll