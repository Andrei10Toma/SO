CC = cl
CFLAGS = /MD

build: so-cpp.exe

so-cpp.exe: so-cpp.obj hash_table.obj vector.obj utils.obj
	$(CC) /o $@ $(CFLAGS) $**

so-cpp.obj: so-cpp.c
	$(CC) /c $**
hash_table.obj: hash_table.c
	$(CC) /c $**
vector.obj: vector.c
	$(CC) /c $**
utils.obj: utils.c
	$(CC) /c $**

clean:
	del *.exe *.obj
