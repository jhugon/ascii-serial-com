
srcobjs := src/circular_buffer.o src/ascii_serial_com.o src/ascii_serial_com_device.o src/ascii_serial_com_register_block.o src/ascii_serial_com_register_pointers.o \
			 src/crc_16_dnp_crcmod.o src/crc_16_dnp_bbb.o src/crc_16_dnp_bbf.o src/crc_16_dnp_tbl4bit.o
ifeq ($(platform),native)
  srcobjs := $(srcobjs) src/circular_buffer_io_fd_poll.o
endif
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
include src/avr/Makefile
include src/arm/Makefile
