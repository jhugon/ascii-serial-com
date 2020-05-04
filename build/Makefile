
.PHONY: all clean native_gcc_debug native_gcc_opt avr_gcc_debug avr_gcc_opt

all: native_gcc_debug native_gcc_opt avr_gcc_debug avr_gcc_opt

native_gcc_opt:
	mkdir -p native_gcc_opt
	cp ../src/Makefile native_gcc_opt/.
	cp ../src/*.c native_gcc_opt/.
	$(MAKE) -C native_gcc_opt CC=gcc build_type=opt

native_gcc_debug:
	mkdir -p native_gcc_debug
	cp ../src/Makefile native_gcc_debug/.
	cp ../src/*.c native_gcc_debug/.
	$(MAKE) -C native_gcc_debug CC=gcc build_type=debug

avr_gcc_opt:
	mkdir -p avr_gcc_opt
	cp ../src/Makefile avr_gcc_opt/.
	cp ../src/*.c avr_gcc_opt/.
	$(MAKE) -C avr_gcc_opt CC=avr-gcc build_type=opt

avr_gcc_debug:
	mkdir -p avr_gcc_debug
	cp ../src/Makefile avr_gcc_debug/.
	cp ../src/*.c avr_gcc_debug/.
	$(MAKE) -C avr_gcc_debug CC=avr-gcc build_type=debug

clean:
	-rm -fr native_gcc_opt
	-rm -fr native_gcc_debug
	-rm -fr avr_gcc_opt
	-rm -fr avr_gcc_debug

