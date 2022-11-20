CURRENTDIR=$(shell pwd)
TOPDIR=$(abspath $(CURRENTDIR)./../..)


vpath=$(subst $(TOPDIR),,$(CURRENTDIR))

OUTPUTDIR=$(abspath $(CURRENTDIR)/../../build/$(vpath))

TARGET=elfParser

INCLUDEPATH=$(CURRENTDIR)/include -I $(CURRENTDIR)/../common/include

EXTERNAL = -L $(OUTPUTDIR)/../common -ldataOprate

MAKE=make

AR=ar cr

# Get the source file, target file and dependent file name
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,$(OUTPUTDIR)/%.o,$(SRCS))
DEPS=$(patsubst %.c,$(OUTPUTDIR)/%.d,$(SRCS))

################ all #################
# The TARGET and LIB variables here are given by the 
# Makefiles of the main module and the sub module respectively. 
# If there is no such variable, no rule will be made

.PHONY=all
all:$(TARGET)

$(TARGET):$(OBJS) 
	@echo "$(shell pwd) TARGET=$(TARGET) SUBDIRS=$(SUBDIRS) OBJS=$(OBJS)"
	@echo "Compiling $(TARGET)"
	mkdir -p $(OUTPUTDIR)
	$(CC) $^ -o $(OUTPUTDIR)/$@ $(LDFLAGS) $(EXTERNAL)

$(OUTPUTDIR)/%.o:%.c
	@echo "Compiling $<"
	mkdir -p $(OUTPUTDIR)
	$(CC) -c $(CURRENTDIR)/$< -o $@ $(CFLAGS) -I $(INCLUDEPATH)

###########  生成和包含依赖文件include .d  ###############
#这里表示当make clean时就不进行这个操作了
ifneq ($(MAKECMDGOALS),clean)
include $(DEPS)
endif

#注意：这里人为指定了依赖关系中的目标为.o和.d，为的是在h文件更新时，依赖文件也能更新
$(OUTPUTDIR)/%.d:%.c
	@echo "creating $@"
	mkdir -p $(OUTPUTDIR)
	$(CC) $(CURRENTDIR)/$< -MM -MF $@ -MT $(OUTPUTDIR)/$*.o -MT $(OUTPUTDIR)/$*.d -I $(INCLUDEPATH)

########### clean ###################
.PHONY=clean
clean:
	@echo "$(shell pwd) cleaning "
	for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir clean||exit 1;\
	done
	rm -f $(OBJS) $(TARGET) $(DEPS)