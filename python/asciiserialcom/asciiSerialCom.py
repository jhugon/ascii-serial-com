"""
ASCII Serial Com Python Interface

"""

import io
import math
import logging
import trio
from trio._file_io import AsyncIOWrapper

from .errors import *
from .helpers import (
    check_register_content,
    check_register_number,
    convert_to_hex,
    convert_from_hex,
    frame_from_stream,
)
from .circularBuffer import Circular_Buffer_Bytes
from .message import ASC_Message

from typing import cast, Optional, Union, Any


async def send_message(
    fout, asciiSerialComVersion: bytes, appVersion: bytes, command: bytes, data: bytes
) -> None:
    """
    Low-level message send command
    Does not check if command is defined command or if it was received successfully

    command: single byte lower-case letter command/message type
    data: bytes or None
    """
    msg = ASC_Message(asciiSerialComVersion, appVersion, command, data)
    message = msg.get_packed()
    logging.info("sent:          {}".format(msg))
    await fout.write(message)
    await fout.flush()


class Ascii_Serial_Com:
    asciiSerialComVersion: bytes
    appVersion: bytes
    registerBitWidth: int
    send_w: Optional[trio.abc.SendChannel] = None
    send_r: Optional[trio.abc.SendChannel] = None
    send_s: Optional[trio.abc.SendChannel] = None
    write_s: Optional[Any] = None
    recv_w: Optional[trio.abc.ReceiveChannel] = None
    recv_r: Optional[trio.abc.ReceiveChannel] = None
    send_all_received_channel: Optional[trio.abc.SendChannel] = None
    send_all_received_channel_copy: bool = True
    buf: Circular_Buffer_Bytes = Circular_Buffer_Bytes(128)

    def __init__(
        self,
        nursery: trio.Nursery,
        fin,
        fout,
        registerBitWidth: int,
        asciiSerialComVersion: bytes = b"0",
        appVersion: bytes = b"0",
    ) -> None:
        if len(asciiSerialComVersion) != 1:
            raise Exception(
                f"asciiSerialComVersion must be a single byte not {asciiSerialComVersion:!r}"
            )
        if len(appVersion) != 1:
            raise Exception(f"appVersion must be a single byte not {appVersion:!r}")
        self.fin = fin
        self.fout = fout
        self.asciiSerialComVersion = asciiSerialComVersion
        self.appVersion = appVersion
        self.registerBitWidth = registerBitWidth
        nursery.start_soon(self._receiver_task)

    async def send_message(self, command: bytes, data: bytes) -> None:
        """
        Low-level message send command
        Does not check if command is defined command or if it was received successfully

        command: single byte lower-case letter command/message type
        data: bytes or None
        """
        await send_message(
            self.fout, self.asciiSerialComVersion, self.appVersion, command, data
        )

    async def read_register(self, regnum: int) -> int:
        """
        Read register on device

        Probably want a timeout on this just in case the device never replies (or it gets garbled)

        regnum: an integer register number from 0 to 0xFFFF

        returns register content as int
        """

        regnum_hex = check_register_number(regnum)
        await self.send_message(b"r", regnum_hex)
        self.send_r, self.recv_r = trio.open_memory_channel(
            0
        )  # now receiver task will pass them into these channels
        # read all messages in queue until one is correct or get cancelled
        while True:
            msg_raw = await self.recv_r.receive()
            msg = cast(ASC_Message, msg_raw)
            if msg is None:
                continue
            splitdata = msg.data.split(b",")
            try:
                rec_regnum, rec_value = splitdata
            except ValueError:
                await self.send_r.aclose()
                await self.recv_r.aclose()
                self.send_r = None
                self.recv_r = None
                raise BadDataError(
                    f"read response data, {msg!r}, can't be split into a reg num and reg val (no comma!)"
                )
            else:
                if int(rec_regnum, 16) == int(regnum_hex, 16):
                    await self.send_r.aclose()
                    await self.recv_r.aclose()
                    self.send_r = None
                    self.recv_r = None
                    return convert_from_hex(rec_value)

    async def write_register(self, regnum: int, content: Union[bytes, int]) -> None:
        """
            write register on device

            Probably want a timeout on this just in case the device never replies (or it gets garbled)

            regnum: an integer register number

            content: bytes to write to the regnum or an integer.
                The integer is converted to little-endian bytes,
                and negative integers aren't allowed.
            """
        regnum_hex = check_register_number(regnum)
        content_hex = check_register_content(content, self.registerBitWidth)
        data = regnum_hex + b"," + content_hex
        await self.send_message(b"w", data)
        self.send_w, self.recv_w = trio.open_memory_channel(
            0
        )  # now receiver task will pass them into these channels
        # read all messages in queue until one is correct or get cancelled
        while True:
            msg_raw = await self.recv_w.receive()
            msg = cast(ASC_Message, msg_raw)
            if msg.command == b"w":
                try:
                    msg_regnum = int(msg.data, 16)
                except ValueError:
                    await self.send_w.aclose()
                    await self.recv_w.aclose()
                    self.send_w = None
                    self.recv_w = None
                    raise BadDataError(
                        f"write response data, {msg.data!r}, isn't a valid register number"
                    )
                else:
                    if msg_regnum == int(regnum_hex, 16):
                        await self.send_w.aclose()
                        await self.recv_w.aclose()
                        self.send_w = None
                        self.recv_w = None
                        return

    def forward_received_s_messages_to(
        self, channel: Union[None, trio.abc.SendChannel, io.IOBase, AsyncIOWrapper]
    ) -> None:
        """
        Send all future streaming frame "s" command messages to the given channel.

        If channel is None, then "s" command messages are dropped.
        """
        self.send_s = None
        self.write_s = None
        if channel is None:
            self.send_s = channel
        elif isinstance(channel, trio.abc.SendChannel):
            self.send_s = channel
        elif isinstance(channel, AsyncIOWrapper):
            channel.wrapped.write
            self.write_s = channel
        elif isinstance(channel, io.IOBase):
            self.write_s = trio.wrap_file(channel)
        else:
            raise Exception("Channel wrong type")

    def forward_all_received_messages_to(
        self, channel: Optional[trio.abc.SendChannel], copy: bool = False
    ) -> None:
        """
        This lets you send all recieved messages to a channel for handling. Useful for testing and debugging.

        If copy is True, then sends each message to channel and also sends them to the channel for each command (if present).

        If copy is False, then sends each message to channel only. This will break some commands like write_register and read_register.

        If channel is None, then resets to normal behavior
        """
        self.send_all_received_channel = channel
        self.send_all_received_channel_copy = copy

    async def _receiver_task(self) -> None:
        """
        This is the task that handles reading from the serial link (self.fin)
        and then puts ASC_Message's in queues
        """
        while True:
            msg = await self._receive_message()
            if msg:
                if self.send_all_received_channel:
                    await self.send_all_received_channel.send(msg)
                    if not self.send_all_received_channel_copy:
                        continue  # skip all of the other sends
                if msg.command == b"w" and self.send_w:
                    await self.send_w.send(msg)
                elif msg.command == b"r" and self.send_r:
                    await self.send_r.send(msg)
                elif msg.command == b"s":
                    if self.send_s:
                        logging.debug(f"About to send to send_s {msg}")
                        await self.send_s.send(msg)
                    elif self.write_s:
                        await self.write_s.write(msg.get_packed())
                elif msg.command == b"e":
                    logging.warning(f"Error message received: {msg}")
                else:
                    pass

    async def _receive_message(self) -> Optional[ASC_Message]:
        """
        You usually won't need this, instead use receiver_loop

        fin: file-like object to read from

        uses Circular_Buffer_Bytes to keep track of input within and between invocations of receive_message

        if no frame is received, all members ASC_Message will be None

        """
        frame = await frame_from_stream(self.fin, self.buf)
        if frame is None:
            logging.debug("received frame is None")
            return None
        logging.debug("received: {!r}".format(frame))
        msg = ASC_Message.unpack(frame)
        if msg.ascVersion != self.asciiSerialComVersion:
            raise AsciiSerialComVersionMismatchError(
                "Message version: {!r} Expected version: {!r}".format(
                    msg.ascVersion, self.asciiSerialComVersion
                )
            )
        if msg.appVersion != self.appVersion:
            raise ApplicationVersionMismatchError(
                "Message version: {!r} Expected version: {!r}".format(
                    msg.appVersion, self.appVersion
                )
            )
        logging.info("received: {}".format(msg))
        return msg
