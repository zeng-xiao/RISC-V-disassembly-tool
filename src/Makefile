####################  Makefile.env  #######################
# Top level pattern, include by Makefile of child directory
# in which variable like TOPDIR, TARGET or LIB may be needed
###########################################################

CC=gcc
MAKE=make

AR=ar cr

CFLAGS=-g3 -ggdb -gdwarf -O0 -Werror

#获取下一层目录名称,以便后续循环调用下一层目录makefile
#其中exclude_dirs代表不需要进行检索的目录，该变量由顶层makefile给出
dirs:=$(shell find . -maxdepth 1 -type d)
dirs:=$(basename $(patsubst ./%,%,$(dirs)))
dirs:=$(filter-out $(exclude_dirs),$(dirs))
SUBDIRS:=$(dirs)

#这里的目的是将主模块的路径放在最后，以便其余子模块先编译
#基本原理是先给出主模块的目录，在检索到下一层级包含有主模块目录时，
#将主模块目录先从其中过滤出来，然后在拼接到末尾
SOURCEDIR=readelf
TMPDIR:=$(filter $(SOURCEDIR),$(dirs))

ifeq ($(TMPDIR),$(SOURCEDIR))
SUBDIRS:=$(filter-out $(SOURCEDIR),$(dirs)) $(SOURCEDIR)
endif

#获取源文件，目标文件和依赖文件名
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
DEPS=$(patsubst %.c,%.d,$(SRCS))

################ all #################
#这里的TARGET，LIB变量分别由主模块和子模块的makefile给出，若无则不进行规则
.PHONY=all
all:$(TARGET) $(LIB) subdirs


#该规则是这个makefile的重点，代表层级调用，循环调用每个子目录下的makefile
subdirs:$(SUBDIRS)
	@echo "$(shell pwd) TARGET=$(TARGET) LIB=$(LIB) SUBDIRS=$(SUBDIRS)"
	for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir all||exit 1;\
	done

$(TARGET):$(OBJS)
	@echo "$(shell pwd) TARGET=$(TARGET) LIB=$(LIB) SUBDIRS=$(SUBDIRS)"
	@echo "Compiling $(TARGET)"
	$(CC) $^ -o $@ $(LDFLAGS) -L $(LIBPATH)
	cp $(TARGET) $(BINPATH)

%.o:%.c
	@echo "Compiling $<"
	$(CC) -c  $< -o $@ $(CFLAGS) -I $(INCLUDEPATH)

###########  生成和包含依赖文件include .d  ###############
#这里表示当make clean时就不进行这个操作了
ifneq ($(MAKECMDGOALS),clean)
include $(DEPS)
endif

#注意：这里人为指定了依赖关系中的目标为.o和.d，为的是在h文件更新时，依赖文件也能更新
%.d:%.c
	@echo "creating $@"
	$(CC) $< -MM -MF $@ -MT $*.o -MT $*.d -I $(INCLUDEPATH)

########### clean ###################
.PHONY=clean
clean:
	@echo "$(shell pwd) cleaning "
	for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir clean||exit 1;\
	done
	rm -f $(OBJS) $(TARGET) $(LIB) $(DEPS)