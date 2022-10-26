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

.PHONY=clean
clean:
	make -f $(TOPDIR)/Makefile.env clean
	rm -rf build bin