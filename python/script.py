#!/usr/bin/env python3
"""
Shell for interacting through ASCII Serial Com
"""

import sys
import os.path
from asciiserialcom.asciiSerialCom import Ascii_Serial_Com
from asciiserialcom.ascErrors import *


def main():
    inFname = "/dev/ttyACM0"
    outFname = "/dev/ttyACM0"
    registerBitWidth = 8

    print(f"Input file name: {inFname}")
    print(f"outFname file name: {outFname}")
    print(f"Register bits: {registerBitWidth}")

    # with open(outFname, "wb", buffering=0) as fout:
    #    with open(inFname, "rb", buffering=0) as fin:
    #        asc = Ascii_Serial_Com(
    #            fin, fout, registerBitWidth, True
    #        )
    with open(outFname, "r+b", buffering=0) as fout:
        asc = Ascii_Serial_Com(fout, fout, registerBitWidth, printMessages=True)
        try:
            result = asc.read_register(4)
        except ASCErrorBase as e:
            printError(e)
        else:
            bitWidth = asc.getRegisterBitWidth()
            hexWidth = str(int(math.ceil(bitWidth / 4)))
            decWidth = str(int(math.ceil(bitWidth * math.log10(2))))
            bitWidth = str(bitWidth)
            formatstr = (
                "{0:"
                + decWidth
                + "d} = 0x{0:0"
                + hexWidth
                + "X} = {0:#0"
                + bitWidth
                + "b}"
            )
            print(formatstr.format(int(result, 16)))


if __name__ == "__main__":
    main()
