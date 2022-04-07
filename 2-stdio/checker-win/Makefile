CC = cl
CFLAGS = /DDLL_EXPORTS

build: so_stdio.dll

so_stdio.dll: so_stdio.obj
	link /dll /out:$@ $** $(CFLAGS)

clean:
	del *.obj so_stdio.dll