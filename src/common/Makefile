TOPDIR=./../..

TARGET=readelf-riscv

LIB=libdataOprate.a

INCLUDEPATH=./include

LIBPATH=$(TOPDIR)/lib

include $(TOPDIR)/Makefile.env

$(LIB):$(OBJS)
	@echo "$(shell pwd) TARGET=$(TARGET) LIB=$(LIB) SUBDIRS=$(SUBDIRS) $(OBJS)"
	$(AR) $@ $(OBJS)