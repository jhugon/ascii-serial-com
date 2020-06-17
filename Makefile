
.PHONY: cppcheck default doc

## Default rule

default:
	$(MAKE) -C build native_gcc_debug
	$(MAKE) -C build native_clang_debug

cppcheck:
	$(MAKE) -C src cppcheck

doc:
	$(MAKE) -C doc

## If none of above pass goal on to build dir goals:
.DEFAULT:
	$(MAKE) -C build $(MAKECMDGOALS)
