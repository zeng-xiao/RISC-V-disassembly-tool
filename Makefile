CFLAGS = -g3 -ggdb -gdwarf -O0 -Werror

.PHONY : readelf-riscv clean

readelf-riscv : lib/readelf/readelf.c
	gcc $(CFLAGS) -I./lib/readelf/include -I../common/include lib/readelf/readelf.c lib/readelf/elfHeader.c -o readelf-riscv


run-readelf-riscv : readelf-riscv
	./readelf-riscv ~/code/riscv/dwarf_relocations/add.c.S.o

clean :
	-rm -rf readelf-riscv
