
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

include src/Makefile

#$(info alllibs is ${alllibs})
#$(info exes is ${exes})
#$(info allobjs is ${allobjs})
#$(info testexes is ${testexes})

.PHONY: all_later
all_later: $(alllibs) $(exes) $(testexes)

$(allobjs): %.o: %.c

$(exes): %: %.o $(alllibs)

######################################

outexes := $(addprefix $(builddir),$(notdir $(exes)))
outtestexes := $(addprefix $(builddir),$(notdir $(testexes)))

$(builddir):
	mkdir -p $(builddir)

$(outexes): $(builddir)%: src/native/% | $(builddir)
	cp $^ $(builddir)

$(outtestexes): $(builddir)%: src/test/% | $(builddir)
	cp $^ $(builddir)

$(builddir)/libasciiserialcom.a: src/libasciiserialcom.a | $(builddir)
	cp $^ $@

$(builddir)/libthrowtheswitch.a: src/externals/libthrowtheswitch.a | $(builddir)
	cp $^ $@

.PHONY: install
install: $(builddir)/libthrowtheswitch.a $(builddir)/libasciiserialcom.a $(outtestexes) $(outexes) all

######################################

runouttestexes := $(addsuffix _runTest,$(outtestexes))

.PHONY: test
test: $(runouttestexes)

.PHONY: $(runouttestexes)
$(runouttestexes): %_runTest: %
	$(abspath $*)

######################################

allgcno := $(addsuffix .gcno, $(basename $(allobjs)))
allgcda := $(addsuffix .gcda, $(basename $(allobjs)))
#allgcov := $(addsuffix .gcov, $(basename $(allobjs)))

.PHONY: clean
clean:
	rm -rf $(allobjs)
	rm -rf $(alllibs)
	rm -rf $(exes)
	rm -rf $(testexes)
	rm -rf $(allgcno)
	rm -rf $(allgcda)
	#rm -rf $(allgcov)
