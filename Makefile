CFLAGS = -g -g3 -ggdb -gdwarf-4 -O0

.PHONY : readelf-riscv clean

readelf-riscv : lib/readelf/readelf.c
	gcc $(CFLAGS) lib/readelf/readelf.c -o readelf-riscv
clean :
	-rm -rf readelf-riscv
