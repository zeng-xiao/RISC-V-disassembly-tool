TOPDIR=$(shell pwd)

BINPATH=$(TOPDIR)/bin
BUILDPATH=$(TOPDIR)/build
LIBPATH=$(TOPDIR)/lib
export BINPATH BUILDPATH

exclude_dirs= bin build include
export exclude_dirs

.PHONY=all
all: $(BINPATH) $(BUILDPATH)
	make -f $(TOPDIR)/Makefile.env all

$(BUILDPATH):
	mkdir -p $(BUILDPATH)

$(BINPATH):
	mkdir -p $(BINPATH)

$(LIBPATH):
	mkdir -p $(LIBPATH)

readelf : $(BINPATH)/readelf-riscv
	$(BINPATH)/readelf-riscv /home/user/code/riscv/dwarf_relocations/add.c.S.o

.PHONY=clean
clean:
	make -f $(TOPDIR)/Makefile.env clean
	rm -rf build bin
