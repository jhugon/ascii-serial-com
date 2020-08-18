
atmega328pfirmware=src/avr/arduino_uno_blink src/avr/arduino_uno_blink_interrupt src/avr/arduino_uno_char_loopback src/avr/arduino_uno_cb_loopback src/avr/arduino_uno_asc_loopback src/avr/arduino_uno_dummy_register_block src/avr/arduino_uno_register_pointers

atmega2560firmware=src/avr/arduino_mega2560_blink

CFLAGSatmega328p=$(subst -mmcu=avr5,-mmcu=atmega328p,$(CFLAGS))
CFLAGSatmega2560=$(subst -mmcu=avr6,-mmcu=atmega2560,$(CFLAGS))

ifeq ($(platform),avr5) # atmega328 and 328p
  avrfirmware+=$(atmega328pfirmware)
  outavrfirmware+=$(addprefix $(builddir),$(notdir $(avrfirmware)))
endif
ifeq ($(platform),avr6) # atmega2560
  avrfirmware+=$(atmega2560firmware)
  outavrfirmware+=$(addprefix $(builddir),$(notdir $(avrfirmware)))
endif

$(atmega328pfirmware): %: %.c src/libasciiserialcom.a src/externals/libthrowtheswitch.a
	avr-gcc $(CFLAGSatmega328p) -o $@ $^

$(atmega2560firmware): %: %.c src/libasciiserialcom.a src/externals/libthrowtheswitch.a
	avr-gcc $(CFLAGSatmega2560) -o $@ $^

$(outavrfirmware): $(builddir)%: src/avr/% | $(builddir)
	cp $^ $@
