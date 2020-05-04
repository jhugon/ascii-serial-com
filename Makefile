
.PHONY: all clean cppcheck

all:
	$(MAKE) -C build

clean:
	$(MAKE) -C build clean

cppcheck:
	$(MAKE) -C src cppcheck
	
