avrobjs=src/avr/avr_uart.o

atmega328pfirmware=src/avr/arduino_uno_blink src/avr/arduino_uno_char_loopback

atmega2560firmware=src/avr/arduino_mega2560_blink

CFLAGSatmega328p=$(subst -mmcu=avr5,-mmcu=atmega328p,$(CFLAGS))
CFLAGSatmega2560=$(subst -mmcu=avr6,-mmcu=atmega2560,$(CFLAGS))

ifeq ($(platform),avr5) # atmega328 and 328p
  avrfirmware+=$(atmega328pfirmware)
  outavrfirmware+=$(addprefix $(builddir),$(notdir $(avrfirmware)))
  alllibs := $(alllibs) src/avr/libascavr.a
endif
ifeq ($(platform),avr6) # atmega2560
  avrfirmware+=$(atmega2560firmware)
  outavrfirmware+=$(addprefix $(builddir),$(notdir $(avrfirmware)))
  alllibs := $(alllibs) src/avr/libascavr.a
endif

$(atmega328pfirmware): %: %.c src/avr/libascavr.a
	avr-gcc $(CFLAGSatmega328p) -o $@ $^ src/avr/libascavr.a

$(atmega2560firmware): %: %.c src/avr/libascavr.a
	avr-gcc $(CFLAGSatmega2560) -o $@ $^ src/avr/libascavr.a

$(outavrfirmware): $(builddir)%: src/avr/% | $(builddir)
	cp $^ $@

src/avr/libascavr.a: $(avrobjs)
	$(AR) rcs $@ $^
