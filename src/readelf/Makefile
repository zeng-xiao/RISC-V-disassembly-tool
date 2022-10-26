TOPDIR=./../..

TARGET=readelf-riscv

LIB=libreadelf.a



INCLUDEPATH=./include ../common/include

LIBPATH=$(TOPDIR)/lib

include $(TOPDIR)/Makefile.env

OBJSNO=$(filter-out readelf.o, $(OBJS))


$(LIB):$(OBJSNO)
	@echo "$(shell pwd) TARGET=$(TARGET) LIB=$(LIB) SUBDIRS=$(SUBDIRS) $(OBJSNO)"
	$(AR) $@ $(OBJSNO)
	mv $@ $^ $(BUILDPATH)