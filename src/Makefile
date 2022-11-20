dirs:=$(shell find . -maxdepth 1 -type d)
dirs:=$(basename $(patsubst ./%,%,$(dirs)))
dirs:=$(filter-out $(exclude_dirs),$(dirs))
SUBDIRS:=$(dirs)

MAINDIR=elfParser
SUBDIRS:=$(filter-out $(MAINDIR),$(dirs)) $(MAINDIR)

#该规则是这个makefile的重点，代表层级调用，循环调用每个子目录下的makefile
subdirs:$(SUBDIRS)
	@echo "$(shell pwd) TARGET=$(TARGET) LIB=$(LIB) SUBDIRS=$(SUBDIRS)"
	for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir all||exit 1;\
	done

################ all #################
#这里的TARGET，LIB变量分别由主模块和子模块的makefile给出，若无则不进行规则
.PHONY=all
all: subdirs

########### clean ###################
.PHONY=clean
clean:
	@echo "$(shell pwd) cleaning "
	for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir clean||exit 1;\
	done