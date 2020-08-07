
atmega328pfirmware=src/avr/arduino_uno_blink

CFLAGSatmega328p=$(subst -mmcu=avr5,-mmcu=atmega328p,$(CFLAGS))

ifeq ($(platform),avr5) # atmega328 and 328p
  avrfirmware+=$(atmega328pfirmware)
  outavrfirmware+=$(addprefix $(builddir),$(notdir $(avrfirmware)))
endif

$(atmega328pfirmware): %: %.c
	avr-gcc $(CFLAGSatmega328p) -o $@ $^

$(outavrfirmware): $(builddir)%: src/avr/% | $(builddir)
	cp $^ $@


#src/avr/blink: src/avr/blink.c
#	avr-gcc $(CFLAGSavr328p) -o $@ $^
#$(builddir)/blink: src/avr/blink | $(builddir)
#	cp $^ $(builddir)
