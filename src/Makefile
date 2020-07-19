
srcobjs := src/circular_buffer.o src/ascii_serial_com.o src/compute_crc.o
allobjs := $(allobjs) $(srcobjs)
#exes := $(exes)
alllibs := $(alllibs) src/libasciiserialcom.a

src/libasciiserialcom.a: $(srcobjs)
	$(AR) rcs $@ $^

include src/externals/Makefile
ifeq ($(platform),native)
  include src/native/Makefile
  include src/unit_test/Makefile
endif
