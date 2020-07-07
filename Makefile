
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

builddir := build_$(target_type)/
$(info builddir is ${builddir})

.PHONY: all
all: all_later

include flags.mk

allobjs :=
alllibs :=
allexes :=
include src/Makefile

$(info alllibs is ${alllibs})
$(info allexes is ${allexes})
$(info allobjs is ${allobjs})

.PHONY: all_later
all_later: $(alllibs) $(allexes)

$(allobjs): %.o: %.c

######################################

outtestexes := $(builddir)$(notdir $(testexes))

$(builddir):
	mkdir -p $(builddir)

$(outtestexes): $(builddir)%: src/test/% | $(builddir)
	cp $^ $(builddir)

$(builddir)/libasciiserialcom.a: src/libasciiserialcom.a | $(builddir)
	cp $^ $@

$(builddir)/libthrowtheswitch.a: src/externals/libthrowtheswitch.a | $(builddir)
	cp $^ $@

.PHONY: install
install: $(builddir)/libthrowtheswitch.a $(builddir)/libasciiserialcom.a $(outtestexes) all

######################################

.PHONY: clean
clean:
	rm -rf $(allobjs)
	rm -rf $(alllibs)
	rm -rf $(allexes)
