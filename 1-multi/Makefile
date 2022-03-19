CC = cl
CFLAGS = /Wall /Wextra /g /MD

all: so-cpp

so-cpp.exe: so-cpp.obj hash_table.obj vector.obj utils.obj
	$(CC) /out:$@ $(CFLAGS) $**

so-cpp.obj: so-cpp.c
hash_table.obj: hash_table.c
vector.obj: vector.c
utils.obj: utils.c

clean: exe_clean obj_clean

exe_clean:
	del so-cpp.exe

obj_clean:
	del *.obj