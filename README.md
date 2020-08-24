# ascii-serial-com

Serial communication library between computers and microcontrollers, FPGAs,
etc. Uses only ASCII. Not the most efficient protocol, but meant to be easy to
read.

## How to flash:

  avrdude -p atmega328p -c arduino -P /dev/ttyACM0 -Uflash:w:build/avr5_gcc_debug/arduino_uno_blink

## Transmit/Receive Data Interface

Will need to be able to transmit and receive messages. This could mean just
putting them in the input and output buffers within ASCII-Serial-Com, and then
having some other functions to actually transmit or receive the bytes from
those buffers into other buffers etc.

May need a polling loop or even software interrupt that says the receive buffer
needs to be parsed for a message.

It seems like having access to the push/pop functions of the buffers would be
desirable. It may be easiest to have a function to give access to the buffers,
and then let buffer methods handle things. Atomic buffer operations may also be
useful to disable interrupts.

### AVR

For transmission: a flag or interrupt will say the (single byte) transmit
buffer is empty. Will need to pop a byte off of the ASCII-Serial-Com output
buffer to put in there. Could also have another buffer in between.

For reception: a flag or interrupt will say the (two byte on 328P) receive
buffer is not empty. Will need to push a byte into the ASCII-Serial-Com input
buffer. Could also have another buffer in between

### STM32

(Need to understand DMA better)

Can do similar thing to AVR or use DMA:

An interrupt will say a DMA buffer is empty/half-full/full. Will need to
pop/push from/to those buffers as appropriate. There may need to be an
abstraction around the DMA buffers to help out.

### Linux Native

Will involve fread or read and fwrite or write. Maybe poll or select.

Maybe poll flags are set on the input and output streams depending on the state
of the ASCII-Serial-Com buffers. Would then need to check if the receive buffer
needs to be parsed after input reads.

## Required Software

gcc, clang, gcovr, python 3.5+

for documentation: doxygen, LaTeX

## The CRC

CRC-16/DNP

  pycrcdnp = pycrc.algorithms.Crc(width = 16, poly = 0x3d65, xor_in = 0, reflect_in = True, reflect_out = True, xor_out=0xFFFF)

  python -m pycrc --width=16 --poly=0x3d65 --xor-in=0 --reflect-in=True --xor-out=0xffff --reflect-out=True --algorithm bbb --generate h -o crc_16_dnp_bbb.h --symbol-prefix=crc_16_dnp_bbb_
  python -m pycrc --width=16 --poly=0x3d65 --xor-in=0 --reflect-in=True --xor-out=0xffff --reflect-out=True --algorithm bbb --generate c -o crc_16_dnp_bbb.c --symbol-prefix=crc_16_dnp_bbb_
  python -m pycrc --width=16 --poly=0x3d65 --xor-in=0 --reflect-in=True --xor-out=0xffff --reflect-out=True --algorithm bbf --generate c -o crc_16_dnp_bbf.c --symbol-prefix=crc_16_dnp_bbf_
  python -m pycrc --width=16 --poly=0x3d65 --xor-in=0 --reflect-in=True --xor-out=0xffff --reflect-out=True --algorithm bbf --generate h -o crc_16_dnp_bbf.h --symbol-prefix=crc_16_dnp_bbf_
  python -m pycrc --width=16 --poly=0x3d65 --xor-in=0 --reflect-in=True --xor-out=0xffff --reflect-out=True --algorithm tbl --table-idx-width=4 --generate h -o crc_16_dnp_tbl4bit.h --symbol-prefix=crc_16_dnp_tbl4bit_
  python -m pycrc --width=16 --poly=0x3d65 --xor-in=0 --reflect-in=True --xor-out=0xffff --reflect-out=True --algorithm tbl --table-idx-width=4 --generate c -o crc_16_dnp_tbl4bit.c --symbol-prefix=crc_16_dnp_tbl4bit_
