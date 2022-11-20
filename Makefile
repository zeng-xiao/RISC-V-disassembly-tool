TOPDIR=$(shell pwd)

OUTPATH=$(TOPDIR)/out
BUILDPATH=$(TOPDIR)/build

#LIBPATH=$(TOPDIR)/lib
#export BINPATH BUILDPATH

exclude_dirs= out build include
export exclude_dirs

$(BUILDPATH):
	mkdir -p $(BUILDPATH)

$(OUTPATH):
	mkdir -p $(OUTPATH)

.PHONY=all
all: $(OUTPATH) $(BUILDPATH)
	make -f $(TOPDIR)/Makefile.mk all

riscv: clean riscvElf riscvElfParser-coremark

x86: clean x86Elf x86ElfParser-coremark

elfParser-add : $(TOPDIR)/src/elfParser/elfParser
	$(TOPDIR)/src/elfParser/elfParser /home/user/code/riscv/dwarf_relocations/add.c.S.o
	#qemu-riscv64 $(TOPDIR)/src/elfParser/elfParser /home/user/code/riscv/dwarf_relocations/add.c.S.o

x86ElfParser-coremark : $(TOPDIR)/src/elfParser/elfParser
	$(TOPDIR)/src/elfParser/elfParser /home/user/riscv-coremark/coremark.bare.riscv.a510Gcc 2>&1 | tee coremark.bare.riscv.a510Gcc.log

riscvElfParser-coremark : $(TOPDIR)/src/elfParser/elfParser
	$(TOPDIR)/src/elfParser/elfParser /home/user/riscv-coremark/coremark.bare.riscv.a510Gcc 2>&1 | tee coremark.bare.riscv.a510Gcc.log
	qemu-riscv64 $(TOPDIR)/src/elfParser/elfParser /home/user/riscv-coremark/coremark.bare.riscv.a510Gcc

riscvElf :
	make CC=/home/user/Downloads/daily/bin/riscv64-unknown-linux-gnu-gcc CFLAGS="-g3 -ggdb -gdwarf -O0 -Werror -march=rv64gc -mabi=lp64d" LDFLAGS="-static" -f $(TOPDIR)/Makefile.mk all

x86Elf :
	make CC=gcc CFLAGS="-g3 -ggdb -gdwarf -O0 -Werror" LDFLAGS="-static" -f $(TOPDIR)/Makefile.mk all

riscvDisassembly : $(TOPDIR)/src/elfParser/elfParser
	/home/user/Downloads/daily/bin/riscv64-unknown-elf-objdump -drswtaxzD -WF -Wf -M no-aliases $(TOPDIR)/src/elfParser/elfParser > $(TOPDIR)/src/elfParser/elfParser.objdump.txt
	/home/user/Downloads/daily/bin/riscv64-unknown-elf-readelf --debug-dump=info --debug-dump=str -aW $(TOPDIR)/src/elfParser/elfParser > $(TOPDIR)/src/elfParser/elfParser.readelf.txt

x86Disassembly : $(TOPDIR)/src/elfParser/elfParser
	objdump -drswtaxzD -WF -Wf -M no-aliases $(TOPDIR)/src/elfParser/readelf-riscv > $(TOPDIR)/src/elfParser/elfParser.objdump.txt
	readelf --debug-dump=info --debug-dump=str -aW $(TOPDIR)/src/elfParser/readelf-riscv > $(TOPDIR)/src/elfParser/elfParser.readelf.txt

.PHONY=clean
clean:
	make -f $(TOPDIR)/Makefile.mk clean
	rm -rf build out