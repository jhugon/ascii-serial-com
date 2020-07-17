# ascii-serial-com

Serial communication library between computers and microcontrollers, FPGAs,
etc. Uses only ASCII. Not the most efficient protocol, but meant to be easy to
read.

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
