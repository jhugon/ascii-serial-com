"""
ASCII Serial Com Python Interface

"""

from __future__ import annotations
import logging

import trio

from .errors import *
from .message import ASC_Message
from .base import Base

from typing import cast, Union


class Host(Base):
    def __init__(
        self,
        nursery: trio.Nursery,
        fin,
        fout,
        registerBitWidth: int,
        asciiSerialComVersion: bytes = b"0",
        appVersion: bytes = b"0",
    ) -> None:
        super().__init__(
            nursery, fin, fout, registerBitWidth, asciiSerialComVersion, appVersion
        )

    async def read_register(self, regnum: int) -> int:
        """
        Read register on device

        Probably want a timeout on this just in case the device never replies (or it gets garbled)

        regnum: an integer register number from 0 to 0xFFFF

        returns register content as int
        """

        regnum_hex = self.check_register_number(regnum)
        await self.send_message(b"r", regnum_hex)
        send_r: trio.abc.SendChannel
        send_r, recv_r = trio.open_memory_channel(0)
        self.forward_received_r_messages_to(send_r)
        # read all messages in queue until one is correct or get cancelled
        while True:
            msg_raw = await recv_r.receive()
            msg = cast(ASC_Message, msg_raw)
            if msg is None:
                continue
            splitdata = msg.data.split(b",")
            try:
                rec_regnum, rec_value = splitdata
            except ValueError:
                await send_r.aclose()
                await recv_r.aclose()
                self.forward_received_r_messages_to(None)
                raise BadDataError(
                    f"read response data, {msg!r}, can't be split into a reg num and reg val (no comma!)"
                )
            else:
                if int(rec_regnum, 16) == int(regnum_hex, 16):
                    await send_r.aclose()
                    await recv_r.aclose()
                    self.forward_received_r_messages_to(None)
                    return self.convert_from_hex(rec_value)

    async def write_register(self, regnum: int, content: Union[bytes, int]) -> None:
        """
        write register on device

        Probably want a timeout on this just in case the device never replies (or it gets garbled)

        regnum: an integer register number

        content: bytes to write to the regnum or an integer.
            The integer is converted to little-endian bytes,
            and negative integers aren't allowed.
        """
        regnum_hex = self.check_register_number(regnum)
        content_hex = self.check_register_content(content)
        data = regnum_hex + b"," + content_hex
        await self.send_message(b"w", data)
        send_w: trio.abc.SendChannel
        send_w, recv_w = trio.open_memory_channel(0)
        self.forward_received_w_messages_to(send_w)
        # read all messages in queue until one is correct or get cancelled
        while True:
            msg_raw = await recv_w.receive()
            msg = cast(ASC_Message, msg_raw)
            if msg.command == b"w":
                try:
                    msg_regnum = int(msg.data, 16)
                except ValueError:
                    await send_w.aclose()
                    await recv_w.aclose()
                    self.forward_received_w_messages_to(None)
                    raise BadDataError(
                        f"write response data, {msg.data!r}, isn't a valid register number"
                    )
                else:
                    if msg_regnum == int(regnum_hex, 16):
                        await send_w.aclose()
                        await recv_w.aclose()
                        self.forward_received_w_messages_to(None)
                        return
