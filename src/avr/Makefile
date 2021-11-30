attiny817firmware=src/avr/attiny817_xplained_blink

attiny3217firmware=src/avr/attiny3217_cnano_blink

atmega328pfirmware=src/avr/arduino_uno_blink src/avr/arduino_uno_blink_interrupt src/avr/arduino_uno_char_loopback src/avr/arduino_uno_cb_loopback src/avr/arduino_uno_asc_loopback src/avr/arduino_uno_dummy_register_block src/avr/arduino_uno_register_pointers src/avr/arduino_uno_write_pattern_to_serial src/avr/arduino_uno_write_message_to_serial src/avr/arduino_uno_asc_write_pattern_to_serial src/avr/arduino_uno_test_device src/avr/arduino_uno_adc_streaming

atmega2560firmware=src/avr/arduino_mega2560_blink

atmega4809firmware=src/avr/atmega4809_cnano_blink

CFLAGSattiny817=$(subst -mmcu=avrxmega3,-mmcu=attiny817,$(CFLAGS))
CFLAGSattiny3217=$(subst -mmcu=avrxmega3,-mmcu=attiny3217,$(CFLAGS))
CFLAGSatmega328p=$(subst -mmcu=avr5,-mmcu=atmega328p,$(CFLAGS))
CFLAGSatmega2560=$(subst -mmcu=avr6,-mmcu=atmega2560,$(CFLAGS))
CFLAGSatmega4809=$(subst -mmcu=avrxmega3,-mmcu=atmega4809,$(CFLAGS))

ifeq ($(platform),avr5) # atmega328 and 328p
  avrfirmware+=$(atmega328pfirmware)
  outavrfirmware+=$(addprefix $(builddir),$(notdir $(avrfirmware)))
endif
ifeq ($(platform),avr6) # atmega2560
  avrfirmware+=$(atmega2560firmware)
  outavrfirmware+=$(addprefix $(builddir),$(notdir $(avrfirmware)))
endif
ifeq ($(platform),avrxmega3) # atmega4809 attiny817 attiny3217
  avrfirmware+=$(attiny817firmware)
  avrfirmware+=$(attiny3217firmware)
  avrfirmware+=$(atmega4809firmware)
  outavrfirmware+=$(addprefix $(builddir),$(notdir $(avrfirmware)))
endif

$(attiny817firmware): %: %.c src/libasciiserialcom.a src/externals/libthrowtheswitch.a
	avr-gcc $(CFLAGSattiny817) -o $@ $^

$(attiny3217firmware): %: %.c src/libasciiserialcom.a src/externals/libthrowtheswitch.a
	avr-gcc $(CFLAGSattiny3217) -o $@ $^

$(atmega328pfirmware): %: %.c src/libasciiserialcom.a src/externals/libthrowtheswitch.a
	avr-gcc $(CFLAGSatmega328p) -o $@ $^

$(atmega2560firmware): %: %.c src/libasciiserialcom.a src/externals/libthrowtheswitch.a
	avr-gcc $(CFLAGSatmega2560) -o $@ $^

$(atmega4809firmware): %: %.c src/libasciiserialcom.a src/externals/libthrowtheswitch.a
	avr-gcc $(CFLAGSatmega4809) -o $@ $^

$(outavrfirmware): $(builddir)%: src/avr/% | $(builddir)
	cp $^ $@
