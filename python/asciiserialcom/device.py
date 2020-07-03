"""
ASCII Serial Com Python Device

This is probably only used for testing, as a microcontroller is usually the
device.
"""

import sys
import os.path
import argparse
import datetime
from asciiserialcom.asciiSerialCom import Ascii_Serial_Com
from asciiserialcom.ascErrors import *


class SimpleTimer(object):
    def __init__(self):
        self.lastReset = datetime.datetime.now()

    def hasBeen(self, nSecs):
        result = (
            self.lastReset + datetime.timedelta(seconds=nSecs) < datetime.datetime.now()
        )
        if result:
            self.lastReset = datetime.datetime.now()
        return result


class Ascii_Serial_Com_Device(object):
    def __init__(self, fin, fout, registerBitWidth, nRegisters):
        self.fin = fin
        self.fout = fout
        self.registerBitWidth = registerBitWidth
        self.nRegisters = nRegisters
        self.asc = Ascii_Serial_Com(fin, fout, registerBitWidth)
        self.registers = [0] * self.nRegisters

    def poll(self, timeout=1.0):
        ascVersion, appVersion, command, data = self.asc.receive_message(timeout)
        if command is None:
            return
        elif command == b"w":
            regNumB, regValB = data.split(b",")
            regNum = self.asc._convert_from_hex(regNumB)
            regVal = self.asc._convert_from_hex(regValB)
            regValOld = self.registers[regNum]
            self.registers[regNum] = regVal
            self.asc.send_message(command, regNumB)
            print(
                f"Write message received: {regNumB} changed from {regValOld:X} to {regValB}"
            )
        elif command == b"r":
            regNum = self.asc._convert_from_hex(data)
            if regNum > 0xFFFF:
                raise BadRegisterNumberError(
                    f"register number, {regNum} = 0x{regNum:04X}, larger than 0xFFFF"
                )
            if regNum >= self.nRegisters:
                raise BadRegisterNumberError(
                    f"Only {self.nRegisters} registers; regNum, {regNum} = 0x{regNum:04X}, too big"
                )
            regVal = self.registers[regNum]
            response = data + b"," + self.asc._convert_to_hex(regVal)
            self.asc.send_message(command, response)
            print(f"Read message received: {regNum} = 0x{regNum:04X} is {regVal}")
        else:
            print(f"Warning: received command '{command}', which is not implemented")

    def printRegisters(self):
        dtstr = datetime.datetime.now().replace(microsecond=0).isoformat(" ")
        if self.registerBitWidth <= 8:
            print("{0:>8}    {1:>19}    {2}".format("Reg Num", "Register Value", dtstr))
        elif self.registerBitWidth <= 16:
            print("{0:>8}    {1:>31}    {2}".format("Reg Num", "Register Value", dtstr))
        else:
            print("{0:>8}    {1:>21}    {2}".format("Reg Num", "Register Value", dtstr))
        for i in range(self.nRegisters):
            val = self.registers[i]
            if self.registerBitWidth <= 8:
                print("{0:3d} 0x{0:02X}    {1:3d} 0x{1:02X} {1:#010b}".format(i, val))
            elif self.registerBitWidth <= 16:
                print("{0:3d} 0x{0:02X}    {1:5d} 0x{1:04X} {1:#018b}".format(i, val))
            else:
                print("{0:3d} 0x{0:02X}    {1:10d} 0x{1:08X}".format(i, val))


def main():
    parser = argparse.ArgumentParser(
        description="ASCII Serial Com Device. Useful as a test device."
    )
    parser.add_argument("fin", help="Path to tty device file")
    parser.add_argument("fout", help="Path to tty device file")
    parser.add_argument(
        "--registerBitWidth",
        "-r",
        type=int,
        default=32,
        help="Device register bit width (default: 32)",
    )
    parser.add_argument(
        "--nRegisters",
        "-n",
        type=int,
        default=16,
        help="Device number of registers (default: 16)",
    )
    parser.add_argument(
        "--timeBetweenPrint",
        "-t",
        type=float,
        default=2.0,
        help="Time between print statements in seconds (default: 2.)",
    )

    args = parser.parse_args()

    inFname = os.path.abspath(args.fin)
    outFname = os.path.abspath(args.fout)
    print(f"Input file name: {inFname}")
    print(f"outFname file name: {outFname}")
    print(f"N registers: {args.nRegisters}")
    print(f"Time between prints: {args.timeBetweenPrint}")

    with open(outFname, "wb", buffering=0) as fout:
        with open(inFname, "rb", buffering=0) as fin:
            dev = Ascii_Serial_Com_Device(
                fin, fout, args.registerBitWidth, args.nRegisters
            )
            dev.printRegisters()
            timer = SimpleTimer()
            while True:
                try:
                    dev.poll()
                except ASCErrorBase as e:
                    printError(e)
                if timer.hasBeen(args.timeBetweenPrint):
                    dev.printRegisters()
