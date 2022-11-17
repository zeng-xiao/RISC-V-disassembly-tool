TOPDIR=$(shell pwd)

#BINPATH=$(TOPDIR)/bin
#BUILDPATH=$(TOPDIR)/build
#LIBPATH=$(TOPDIR)/lib
#export BINPATH BUILDPATH

exclude_dirs= bin build include
export exclude_dirs

.PHONY=all
all: $(BINPATH) $(BUILDPATH)
	make -f $(TOPDIR)/Makefile.env all

$(BUILDPATH):
	mkdir -p $(BUILDPATH)

$(BINPATH):
	mkdir -p $(BINPATH)

readelf : $(TOPDIR)/src/readelf/readelf-riscv
	$(TOPDIR)/src/readelf/readelf-riscv /home/user/code/riscv/dwarf_relocations/add.c.S.o
	#qemu-riscv64 $(TOPDIR)/src/readelf/readelf-riscv /home/user/code/riscv/dwarf_relocations/add.c.S.o

riscvElf :
	make CC=/home/user/Downloads/daily/bin/riscv64-unknown-linux-gnu-gcc CFLAGS="-g3 -ggdb -gdwarf -O0 -Werror -march=rv64gc -mabi=lp64d -static" LDFLAGS="-static"

x86Elf :
	make

riscvDisassembly : $(TOPDIR)/src/readelf/readelf-riscv
	/home/user/Downloads/daily/bin/riscv64-unknown-elf-objdump -drswtaxzD -WF -Wf -M no-aliases $(TOPDIR)/src/readelf/readelf-riscv > $(TOPDIR)/src/readelf/readelf-riscv.objdump.txt
	/home/user/Downloads/daily/bin/riscv64-unknown-elf-readelf --debug-dump=info --debug-dump=str -aW $(TOPDIR)/src/readelf/readelf-riscv > $(TOPDIR)/src/readelf/readelf-riscv.readelf.txt

x86Disassembly : $(TOPDIR)/src/readelf/readelf-riscv
	objdump -drswtaxzD -WF -Wf -M no-aliases $(TOPDIR)/src/readelf/readelf-riscv > $(TOPDIR)/src/readelf/readelf-riscv.objdump.txt
	readelf --debug-dump=info --debug-dump=str -aW $(TOPDIR)/src/readelf/readelf-riscv > $(TOPDIR)/src/readelf/readelf-riscv.readelf.txt

.PHONY=clean
clean:
	make -f $(TOPDIR)/Makefile.env clean
	rm -rf build bin
