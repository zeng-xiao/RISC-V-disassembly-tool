objects = hello.o
CFLAGS = -g -g3 -ggdb -gdwarf-4 -O0

.PHONY : readelf clean
readelf :
	gcc lib/readelf/readelf.c -o readelf
clean :
	-rm main $(objects) .gdbinit *.txt *.log *.so *.o core
