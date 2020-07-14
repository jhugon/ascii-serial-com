
srcobjs := src/functest.o src/circular_buffer.o src/ascii_serial_com.o src/compute_crc.o\
			 src/ascii_serial_com_dummy_loopback_device.o
allobjs := $(allojbs) $(srcobjs)
exes := $(exes) src/ascii_serial_com_dummy_loopback_device
alllibs := $(alllibs) src/libasciiserialcom.a

src/libasciiserialcom.a: $(srcobjs)
	$(AR) rcs $@ $^

include src/externals/Makefile
include src/test/Makefile
