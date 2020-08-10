
atmega328pfirmware=src/avr/arduino_uno_blink src/avr/arduino_uno_char_loopback

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

$(atmega328pfirmware): %: %.c
	avr-gcc $(CFLAGSatmega328p) -o $@ $^

$(atmega2560firmware): %: %.c
	avr-gcc $(CFLAGSatmega2560) -o $@ $^

$(outavrfirmware): $(builddir)%: src/avr/% | $(builddir)
	cp $^ $@
