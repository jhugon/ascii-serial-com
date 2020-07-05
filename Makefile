include flags.mk

#####----- Begin Boilerplate for Advanced VPATH
####ifeq (,$(filter build_%,$(notdir $(CURDIR))))
####include target.mk
####else
####VPATH=$(SRCDIR)
####$(info $$VPATH is [${VPATH}])
#####----- End Boilerplate
####
####include src/Makefile
####
#####----- Begin Boilerplate
####endif

allcfiles :=
alllibs :=
allexes :=
include src/Makefile

$(allobjs): %.o: %.c

.PHONY: clean
clean:
	rm -rf $(allobjs)
	rm -rf $(alllibs)
	rm -rf $(allexes)
