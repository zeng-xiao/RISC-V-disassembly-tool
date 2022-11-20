####################  Makefile.env  #######################
# Top level pattern, include by Makefile of child directory
# in which variable like TOPDIR, TARGET or LIB may be needed
###########################################################


# Get the name of the next level directory, so that 
# subsequent cycles can call the Makefile of the next level directory

# Dir of exclude_dirs represents the directory that does not need to 
# be retrieved, and this variable is given by the top-level Makefile

dirs:=$(shell find . -maxdepth 1 -type d)
dirs:=$(basename $(patsubst ./%,%,$(dirs)))
dirs:=$(filter-out $(exclude_dirs),$(dirs))
SUBDIRS:=$(dirs)

################ all #################
# The TARGET and LIB variables here are given by the 
# Makefiles of the main module and the sub module respectively. 
# If there is no such variable, no rule will be made

.PHONY=all
all:subdirs

# This rule is the key point of this Makefile, representing hierarchical
# calls, and circularly calling Makefile under each subdirectory
subdirs:$(SUBDIRS)
	@echo "$(shell pwd) TARGET=$(TARGET) LIB=$(LIB) SUBDIRS=$(SUBDIRS)"
	for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir all || exit 1;\
	done

########### clean ###################
.PHONY=clean
clean:
	@echo "$(shell pwd) cleaning "
	for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir clean||exit 1;\
	done