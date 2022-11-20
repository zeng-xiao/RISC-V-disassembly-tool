TOPDIR=$(shell pwd)

OUTPATH=$(TOPDIR)/out
BUILDPATH=$(TOPDIR)/build

#LIBPATH=$(TOPDIR)/lib
#export BINPATH BUILDPATH

exclude_dirs= out build include
export exclude_dirs

.PHONY=all
all: $(OUTPATH) $(BUILDPATH)
	make -f $(TOPDIR)/Makefile.env all

$(BUILDPATH):
	mkdir -p $(BUILDPATH)

$(OUTPATH):
	mkdir -p $(OUTPATH)

elfParser-add : $(TOPDIR)/src/elfParser/elfParser
	$(TOPDIR)/src/elfParser/elfParser /home/user/code/riscv/dwarf_relocations/add.c.S.o
	#qemu-riscv64 $(TOPDIR)/src/elfParser/elfParser /home/user/code/riscv/dwarf_relocations/add.c.S.o

elfParser-coremark : $(TOPDIR)/src/elfParser/elfParser
	$(TOPDIR)/src/elfParser/elfParser /home/user/riscv-coremark/coremark.bare.riscv.a510Gcc 2>&1 | tee coremark.bare.riscv.a510Gcc.log
	#qemu-riscv64 $(TOPDIR)/src/elfParser/elfParser /home/user/riscv-coremark/coremark.bare.riscv.a510Gcc

riscvElf :
	make CC=/home/user/Downloads/daily/bin/riscv64-unknown-linux-gnu-gcc CFLAGS="-g3 -ggdb -gdwarf -O0 -Werror -march=rv64gc -mabi=lp64d -static" LDFLAGS="-static"

x86Elf :
	make

riscvDisassembly : $(TOPDIR)/src/elfParser/elfParser
	/home/user/Downloads/daily/bin/riscv64-unknown-elf-objdump -drswtaxzD -WF -Wf -M no-aliases $(TOPDIR)/src/elfParser/elfParser > $(TOPDIR)/src/elfParser/elfParser.objdump.txt
	/home/user/Downloads/daily/bin/riscv64-unknown-elf-readelf --debug-dump=info --debug-dump=str -aW $(TOPDIR)/src/elfParser/elfParser > $(TOPDIR)/src/elfParser/elfParser.readelf.txt

x86Disassembly : $(TOPDIR)/src/elfParser/elfParser
	objdump -drswtaxzD -WF -Wf -M no-aliases $(TOPDIR)/src/elfParser/readelf-riscv > $(TOPDIR)/src/elfParser/elfParser.objdump.txt
	readelf --debug-dump=info --debug-dump=str -aW $(TOPDIR)/src/elfParser/readelf-riscv > $(TOPDIR)/src/elfParser/elfParser.readelf.txt

.PHONY=clean
clean:
	make -f $(TOPDIR)/Makefile.env clean
	rm -rf build out
