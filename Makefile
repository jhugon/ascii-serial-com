
ifndef platform
  platform = native
  export platform
endif

ifndef build_type
  build_type = debug
  export build_type
endif

ifndef CC
  CC = gcc
  export CC
endif
ifeq ($(CC),cc)
  CC = gcc
  export CC
endif

target_type := $(platform)_$(CC)_$(build_type)
$(info target_type is ${target_type})

##builddir := build_$(target_type)/
##$(info builddir is ${builddir})

all: all_later

include flags.mk

allobjs :=
alllibs :=
allexes :=
include src/Makefile

$(info alllibs is ${alllibs})
$(info allexes is ${allexes})
$(info allobjs is ${allobjs})

all_later: $(alllibs) $(allexes)

$(allobjs): %.o: %.c

.PHONY: install
install:

.PHONY: clean
clean:
	rm -rf $(allobjs)
	rm -rf $(alllibs)
	rm -rf $(allexes)
