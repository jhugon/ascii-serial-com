"""
ASCII Serial Com Python Device

This is probably only used for testing, as a microcontroller is usually the
device.
"""

import sys
import os.path
import argparse
import datetime
import logging
import trio
from .asciiSerialCom import send_message
from .circularBuffer import Circular_Buffer_Bytes
from .message import ASC_Message
from .helpers import convert_from_hex, convert_to_hex, frame_from_stream
from .errors import *

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
        nursery.start_soon(
            registers.receiver_loop, fin, send_w, send_r, send_s, b"0", b"0"
        )  # type: ignore
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
        logging.info("printRegisters")
        dtstr = datetime.datetime.now().replace(microsecond=0).isoformat(" ")
        if self.registerBitWidth <= 8:
            logging.info(
                "{0:>8}    {1:>19}    {2}".format("Reg Num", "Register Value", dtstr)
            )
        elif self.registerBitWidth <= 16:
            logging.info(
                "{0:>8}    {1:>31}    {2}".format("Reg Num", "Register Value", dtstr)
            )
        else:
            logging.info(
                "{0:>8}    {1:>21}    {2}".format("Reg Num", "Register Value", dtstr)
            )
        for i in range(self.nRegisters):
            val = self.registers[i]
            if self.registerBitWidth <= 8:
                logging.info(
                    "{0:3d} 0x{0:02X}    {1:3d} 0x{1:02X} {1:#010b}".format(i, val)
                )
            elif self.registerBitWidth <= 16:
                logging.info(
                    "{0:3d} 0x{0:02X}    {1:5d} 0x{1:04X} {1:#018b}".format(i, val)
                )
            else:
                logging.info("{0:3d} 0x{0:02X}    {1:10d} 0x{1:08X}".format(i, val))

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
                await send_message(
                    fout,
                    self.asciiSerialComVersion,
                    self.appVersion,
                    msg.command,
                    response,
                )
                logging.info(
                    f"device Read message received: {regNum} = 0x{regNum:04X} is {regVal}"
                )
            else:
                logging.warning(
                    f"device received command '{msg.command}', in read channel"
                )

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
                await send_message(
                    fout,
                    self.asciiSerialComVersion,
                    self.appVersion,
                    msg.command,
                    regNumB,
                )
                logging.info(
                    f"device Write message received: {regNumB} changed from {regValOld:X} to {regValB}"
                )
            else:
                logging.warning(
                    f"Warning: device received command '{msg.command}', in write channel"
                )

    async def receiver_loop(
        self,
        fin,
        queue_w: trio.abc.SendChannel,
        queue_r: trio.abc.SendChannel,
        queue_s: trio.abc.SendChannel,
        asciiSerialComVersion: bytes,
        appVersion: bytes,
    ) -> None:
        """
        This is the task that handles reading from the serial link with file like object fin
        and then puts ASC_Message's in the queues

        All "queue" are the write ends of trio channels i.e. trio.abc.SendChannel
        """
        buf = Circular_Buffer_Bytes(128)
        while True:
            msg = await self.receive_message(
                fin, buf, asciiSerialComVersion, appVersion
            )
            logging.info("device receiver_loop with {}".format(msg))
            if not (msg is None):
                if msg.command == b"w" and queue_w:
                    await queue_w.send(msg)
                elif msg.command == b"r" and queue_r:
                    await queue_r.send(msg)
                elif msg.command == b"s" and queue_s:
                    await queue_s.send(msg)
                else:
                    pass

    async def receive_message(
        self,
        fin,
        buf: Circular_Buffer_Bytes,
        asciiSerialComVersion: bytes,
        appVersion: bytes,
    ) -> Optional[ASC_Message]:
        """
        You usually won't need this, instead use receiver_loop

        fin: file-like object to read from

        uses Circular_Buffer_Bytes to keep track of input within and between invocations of receive_message

        if no frame is received, all members ASC_Message will be None

        """
        frame = await frame_from_stream(fin, buf)
        if frame is None:
            return None
        logging.info("device received message: {}".format(frame))
        msg = ASC_Message.unpack(frame)
        if msg.ascVersion != asciiSerialComVersion:
            raise AsciiSerialComVersionMismatchError(
                "Message version: {!r} Expected version: {!r}".format(
                    msg.ascVersion, asciiSerialComVersion
                )
            )
        if msg.appVersion != appVersion:
            raise ApplicationVersionMismatchError(
                "Message version: {!r} Expected version: {!r}".format(
                    msg.appVersion, appVersion
                )
            )
        return msg


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
