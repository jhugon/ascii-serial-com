"""
ASCII Serial Com Python Device

This is probably only used for testing, as a microcontroller is usually the
device.
"""

import sys
import os.path
import argparse
import datetime
import trio
from .asciiSerialCom import receiver_loop, send_message
from .ascHelpers import convert_from_hex, convert_to_hex
from .ascErrors import *

from typing import Optional, Any, Union


async def deviceLoopOpenFiles(
    finname: str,
    foutname: str,
    registersBitWidth: int,
    nRegisters: int,
    printInterval: float,
) -> None:
    async with await trio.open_file(foutname, "wb", buffering=0) as fout:
        async with await trio.open_file(finname, "rb", buffering=0) as fin:
            await deviceLoop(fin, fout, registersBitWidth, nRegisters, printInterval)


async def deviceLoop(
    fin, fout, registersBitWidth: int, nRegisters: int, printInterval: float
) -> None:
    registers = DeviceRegisters(registersBitWidth, nRegisters)
    async with trio.open_nursery() as nursery:
        send_w: trio.abc.SendChannel
        send_r: trio.abc.SendChannel
        send_w, recv_w = trio.open_memory_channel(0)
        send_r, recv_r = trio.open_memory_channel(0)
        # send_s, recv_s = trio.open_memory_channel(0)
        send_s, recv_s = (
            None,
            None,
        )  # since not implemented yet, don't do anything with these messages

        # have to use type: ignore b/c mypy stub can't deal with so many arguments
        nursery.start_soon(receiver_loop, fin, send_w, send_r, send_s, b"00", b"00")  # type: ignore
        nursery.start_soon(registers.printRegistersLoop, printInterval)
        nursery.start_soon(registers.handle_w_messages, fout, recv_w)
        nursery.start_soon(registers.handle_r_messages, fout, recv_r)


class DeviceRegisters:
    def __init__(self, registerBitWidth: int, nRegisters: int) -> None:
        self.registerBitWidth = registerBitWidth
        self.nRegisters = nRegisters
        self.registers = [0] * self.nRegisters
        self.asciiSerialComVersion = b"0"
        self.appVersion = b"0"

    def printRegisters(self) -> None:
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

    async def printRegistersLoop(self, interval: float) -> None:
        """
        Print the registers every interval seconds
        """

        while True:
            self.printRegisters()
            await trio.sleep(interval)

    async def handle_r_messages(self, fout, recv_r: trio.abc.ReceiveChannel) -> None:
        while True:
            msg = await recv_r.receive()
            if not msg:
                continue
            elif msg.command == b"r":
                regNum = convert_from_hex(msg.data)
                if regNum > 0xFFFF:
                    raise BadRegisterNumberError(
                        f"register number, {regNum} = 0x{regNum:04X}, larger than 0xFFFF"
                    )
                if regNum >= self.nRegisters:
                    raise BadRegisterNumberError(
                        f"Only {self.nRegisters} registers; regNum, {regNum} = 0x{regNum:04X}, too big"
                    )
                regVal = self.registers[regNum]
                response = msg.data + b"," + convert_to_hex(regVal)
                send_message(
                    fout,
                    self.asciiSerialComVersion,
                    self.appVersion,
                    msg.command,
                    response,
                )
                print(f"Read message received: {regNum} = 0x{regNum:04X} is {regVal}")
            else:
                print(f"Warning: received command '{msg.command}', in read channel")

    async def handle_w_messages(self, fout, recv_w: trio.abc.ReceiveChannel) -> None:
        while True:
            msg = await recv_w.receive()
            if not msg:
                continue
            elif msg.command == b"w":
                regNumB, regValB = msg.data.split(b",")
                regNum = convert_from_hex(regNumB)
                regVal = convert_from_hex(regValB)
                regValOld = self.registers[regNum]
                self.registers[regNum] = regVal
                send_message(
                    fout,
                    self.asciiSerialComVersion,
                    self.appVersion,
                    msg.command,
                    regNumB,
                )
                print(
                    f"Write message received: {regNumB} changed from {regValOld:X} to {regValB}"
                )
            else:
                print(f"Warning: received command '{msg.command}', in write channel")


def main() -> None:
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

    trio.run(
        deviceLoopOpenFiles,
        inFname,
        outFname,
        args.registerBitWidth,
        args.nRegisters,
        args.timeBetweenPrint,
    )
