allconfigs=native_gcc_opt native_gcc_debug avr_gcc_opt avr_gcc_debug
allcfiles_src=$(wildcard ../src/*.c)
allcfiles=$(notdir $(allcfiles_src))
allcfiles_configdirs=$(foreach dir,$(allconfigs),$(foreach f,$(allcfiles),$(dir)/$(f)))

#$(info $$allconfigs is [${allconfigs}])
#$(info $$allcfiles is [${allcfiles}])
#$(info $$allcfiles_configdirs is [${allcfiles_configdirs}])

makefiles_configdirs=$(foreach dir,$(allconfigs),$(dir)/Makefile)

.PHONY: all clean copy_files native_gcc_debug native_gcc_opt avr_gcc_debug avr_gcc_opt

all: native_gcc_debug native_gcc_opt avr_gcc_debug avr_gcc_opt

$(allcfiles_configdirs): $(allcfiles_src)

$(makefiles_configdirs): ../src/Makefile

native_gcc_opt: $(patsubst %,native_gcc_opt/%,$(allcfiles)) native_gcc_opt/Makefile
	$(MAKE) -C native_gcc_opt CC=gcc build_type=opt

native_gcc_debug: $(patsubst %,native_gcc_debug/%,$(allcfiles)) native_gcc_debug/Makefile
	$(MAKE) -C native_gcc_debug CC=gcc build_type=debug

avr_gcc_opt: $(patsubst %,avr_gcc_opt/%,$(allcfiles)) avr_gcc_opt/Makefile
	$(MAKE) -C avr_gcc_opt CC=avr-gcc build_type=opt

avr_gcc_debug: $(patsubst %,avr_gcc_debug/%,$(allcfiles)) avr_gcc_debug/Makefile
	$(MAKE) -C avr_gcc_debug CC=avr-gcc build_type=debug

native_gcc_opt/%.c: ../src/%.c
	mkdir -p native_gcc_opt
	cp $< $@

native_gcc_debug/%.c: ../src/%.c
	mkdir -p native_gcc_debug
	cp $< $@

avr_gcc_opt/%.c: ../src/%.c
	mkdir -p avr_gcc_opt
	cp $< $@

avr_gcc_debug/%.c: ../src/%.c
	mkdir -p avr_gcc_debug
	cp $< $@

native_gcc_opt/Makefile: ../src/Makefile
	mkdir -p native_gcc_opt
	cp $< $@

native_gcc_debug/Makefile: ../src/Makefile
	mkdir -p native_gcc_debug
	cp $< $@

avr_gcc_opt/Makefile: ../src/Makefile
	mkdir -p avr_gcc_opt
	cp $< $@

avr_gcc_debug/Makefile: ../src/Makefile
	mkdir -p avr_gcc_debug
	cp $< $@

clean:
	-rm -fr native_gcc_opt
	-rm -fr native_gcc_debug
	-rm -fr avr_gcc_opt
	-rm -fr avr_gcc_debug

