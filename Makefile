TOPDIR=./

exclude_dirs= include bin lib 
export exclude_dirs

.PHONY=all
all:
	make -f $(TOPDIR)/Makefile.env all

.PHONY=clean
clean:
	make -f $(TOPDIR)/Makefile.env clean
	rm -rf build bin