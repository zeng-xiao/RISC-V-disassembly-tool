CURRENTDIR=$(shell pwd)
TOPDIR=$(abspath $(CURRENTDIR)./../..)


vpath=$(subst $(TOPDIR),,$(CURRENTDIR))
OUTPUTDIR=$(abspath $(CURRENTDIR)/../../build/$(vpath))
INCLUDEPATH=$(abspath $(CURRENTDIR)/include)

LIB=libdataOprate.a

MAKE=make

AR=ar cr

# Get the source file, target file and dependent file name
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,$(OUTPUTDIR)/%.o,$(SRCS))
DEPS=$(patsubst %.c,$(OUTPUTDIR)/%.d,$(SRCS))

$(LIB):$(OBJS)
	@echo "$(shell pwd) TARGET=$(TARGET) LIB=$(LIB) SUBDIRS=$(SUBDIRS) $(OBJS)"
	mkdir -p $(OUTPUTDIR)
	$(AR) $(OUTPUTDIR)/$@ $(OBJS)

################ all #################
# The TARGET and LIB variables here are given by the 
# Makefiles of the main module and the sub module respectively. 
# If there is no such variable, no rule will be made
.PHONY=all
all: $(LIB)

$(OUTPUTDIR)/%.o:%.c
	@echo "Compiling $<"
	mkdir -p $(OUTPUTDIR)
	$(CC) -c  $< -o $@ $(CFLAGS) -I $(INCLUDEPATH)

###########  生成和包含依赖文件include .d  ###############
#这里表示当make clean时就不进行这个操作了
ifneq ($(MAKECMDGOALS),clean)
include $(DEPS)
endif

$(OUTPUTDIR)/%.d:%.c
	@echo "creating $@"
	mkdir -p $(OUTPUTDIR)
	$(CC) $< -MM -MF $@ -MT $*.o -MT $*.d -I $(INCLUDEPATH)


.PHONY=clean
clean:
	@echo "$(shell pwd) cleaning "
	for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir clean||exit 1;\
	done
	rm -f $(OBJS) $(LIB) $(DEPS)