
srcobjs := src/functest.o src/circular_buffer.o src/ascii_serial_com.o
allobjs := $(allojbs) $(srcobjs)
allexes := $(allexes)
alllibs := $(alllibs) src/libasciiserialcom.a

src/libasciiserialcom.a: $(srcobjs)
	$(AR) rcs $@ $^

include src/externals/Makefile
include src/test/Makefile
