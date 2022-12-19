TOPDIR=$(shell pwd)

OUTPATH=$(TOPDIR)/out
BUILDPATH=$(TOPDIR)/build

#LIBPATH=$(TOPDIR)/lib
#export BINPATH BUILDPATH
export PATH=$(shell printenv PATH):/home/user/Downloads/tools/daily/bin
#export PATH=$(PATH):/home/user/Downloads/tools/daily/bin

exclude_dirs= out build include
export exclude_dirs

$(BUILDPATH):
	mkdir -p $(BUILDPATH)

$(OUTPATH):
	mkdir -p $(OUTPATH)

.PHONY=all
all: $(OUTPATH) $(BUILDPATH)
	make -f $(TOPDIR)/Makefile.mk all

.PHONY=riscv x86
riscv: clean $(OUTPATH) $(BUILDPATH) riscvElf
x86: clean $(OUTPATH) $(BUILDPATH) x86Elf

riscvAll: riscvElf $(OUTPATH) $(BUILDPATH) riscvElfParser-coremark
x86All: x86Elf $(OUTPATH) $(BUILDPATH) x86ElfParser-coremark

elfParser-add : $(TOPDIR)/build/src/elfParser/elfParser
	$(TOPDIR)/build/src/elfParser/elfParser /home/user/code/riscv/dwarf_relocations/add.c.S.o
	#qemu-riscv64 $(TOPDIR)/build/src/elfParser/elfParser /home/user/code/riscv/dwarf_relocations/add.c.S.o

x86ElfParser-coremark : x86Elf $(TOPDIR)/build/src/elfParser/elfParser
	$(TOPDIR)/build/src/elfParser/elfParser /home/user/riscv-coremark/coremark.bare.riscv.a510Gcc 2>&1 | tee coremark.bare.riscv.a510Gcc.log

riscvElfParser-coremark : riscvElf $(TOPDIR)/build/src/elfParser/elfParser
	$(TOPDIR)/build/src/elfParser/elfParser /home/user/riscv-coremark/coremark.bare.riscv.a510Gcc 2>&1 | tee coremark.bare.riscv.a510Gcc.log
	qemu-riscv64 $(TOPDIR)/build/src/elfParser/elfParser /home/user/riscv-coremark/coremark.bare.riscv.a510Gcc

riscvElf : clean
	echo $(PATH)
	make CC=riscv64-unknown-linux-gnu-gcc CFLAGS="-g3 -ggdb3 -gvariable-location-views -gdwarf-4 -grecord-gcc-switches -O1 -Werror -march=rv64gc -mabi=lp64d" LDFLAGS="-static" -f $(TOPDIR)/Makefile.mk all

x86Elf : clean
	make CC=gcc CFLAGS="-g3 -ggdb -gdwarf -O0 -Werror" LDFLAGS="-static" -f $(TOPDIR)/Makefile.mk all

riscvDisassembly : riscvElf $(TOPDIR)/build/src/elfParser/elfParser
	riscv64-unknown-linux-gnu-objdump -drswtaxzD -WF -Wf -M no-aliases $(TOPDIR)/build/src/elfParser/elfParser > $(TOPDIR)/build/src/elfParser/elfParser.objdump.txt
	riscv64-unknown-linux-gnu-readelf --debug-dump=info --debug-dump=str -aW $(TOPDIR)/build/src/elfParser/elfParser > $(TOPDIR)/build/src/elfParser/elfParser.readelf.txt

x86Disassembly : x86Elf $(TOPDIR)/build/src/elfParser/elfParser
	objdump -drswtaxzD -WF -Wf -M no-aliases $(TOPDIR)/build/src/elfParser/elfParser > $(TOPDIR)/build/src/elfParser/elfParser.objdump.txt
	readelf --debug-dump=info --debug-dump=str -aW $(TOPDIR)/build/src/elfParser/elfParser > $(TOPDIR)/build/src/elfParser/elfParser.readelf.txt

.PHONY=clean
clean:
	make -f $(TOPDIR)/Makefile.mk clean
	rm -rf build out
