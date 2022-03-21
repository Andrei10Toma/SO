CC = cl
CFLAGS = /MD /W3 /D_CRT_SECURE_NO_DEPRECATE

build: so-cpp.exe

so-cpp.exe: so-cpp.obj hash_table.obj vector.obj utils.obj
	$(CC) /o $@ $** $(CFLAGS)

so-cpp.obj: so-cpp.c
hash_table.obj: hash_table.c hash_table.h
vector.obj: vector.c vector.h
utils.obj: utils.c utils.h

clean:
	del *.exe *.obj
