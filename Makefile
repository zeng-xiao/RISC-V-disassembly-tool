TOPDIR=$(shell pwd)

BINPATH=$(TOPDIR)/bin
BUILDPATH=$(TOPDIR)/build
export BINPATH BUILDPATH

exclude_dirs= bin build include
export exclude_dirs

.PHONY=all
all: $(BINPATH) $(BUILDPATH)
	make -f $(TOPDIR)/Makefile.env all

$(BINPATH):
	mkdir -p $(BINPATH)

$(BUILDPATH):
	mkdir -p $(BUILDPATH)

readelf : $(BINPATH)/readelf-riscv
	$(BINPATH)/readelf-riscv /home/user/code/riscv/dwarf_relocations/add.c.S.o

.PHONY=clean
clean:
	make -f $(TOPDIR)/Makefile.env clean
	rm -rf build bin
