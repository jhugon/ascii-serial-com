
.PHONY: all clean Native_gcc_debug Native_gcc_opt

all: Native_gcc_debug Native_gcc_opt

Native_gcc_opt:
	mkdir -p native_gcc_opt
	cp ../src/Makefile native_gcc_opt/.
	cp ../src/*.c native_gcc_opt/.
	$(MAKE) -C native_gcc_opt CC=gcc build_type=opt

Native_gcc_debug:
	mkdir -p native_gcc_debug
	cp ../src/Makefile native_gcc_debug/.
	cp ../src/*.c native_gcc_debug/.
	$(MAKE) -C native_gcc_debug CC=gcc build_type=debug

clean:
	-rm -r native_gcc_opt
	-rm -r native_gcc_debug

