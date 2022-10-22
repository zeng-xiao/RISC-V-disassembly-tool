CFLAGS = -g -g3 -ggdb -gdwarf-4 -O0

.PHONY : readelf clean

readelf : lib/readelf/readelf.c
	gcc $(CFLAGS) lib/readelf/readelf.c -o readelf
clean :
	-rm -rf readelf
