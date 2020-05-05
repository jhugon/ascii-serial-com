
.PHONY: cppcheck default

## Default rule

default:
	$(MAKE) -C build native_gcc_debug

cppcheck:
	$(MAKE) -C src cppcheck

## If none of above pass goal on to build dir goals:
.DEFAULT:
	$(MAKE) -C build $(MAKECMDGOALS)

