"""
ASCII Serial Com Python Interface

Example of using the code in this module:

    async def example_read_reg(fin, fout, regnum: int) -> Optional[bytes]:
        result = None
        with trio.move_on_after(
            1e6
        ) as cancel_scope:  # arbitrarily large; intend parent to actually set timeout
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
                nursery.start_soon(receiver_loop, fin, send_w, send_r, send_s, b"00", b"00")
                result = await read_register(fout, recv_r, regnum)
                cancel_scope.cancel()  # this stops the receiver_loop which is what I setup cancel_scope for in the first place
        return result



"""

import math
import trio

from .ascErrors import *
from .ascHelpers import (
    check_register_content,
    check_register_number,
    convert_to_hex,
    convert_from_hex,
    frame_from_stream,
)
from .circularBuffer import Circular_Buffer_Bytes
from .ascMessage import ASC_Message

from typing import Any, Union


async def read_register(fout, queue_r: trio.abc.ReceiveChannel, regnum: int) -> bytes:
    """
    Read register on device

    queue_r is a read queue where "r" messages are deposited, i.e. a trio.abc.ReceiveChannel

    Assumes reciver_loop is running in another task

    regnum: an integer register number from 0 to 0xFFFF

    returns register content as bytes
    """
    asciiSerialComVersion = b"00"
    appVersion = b"00"

    regnum_hex = check_register_number(regnum)
    await send_message(fout, asciiSerialComVersion, appVersion, b"r", regnum_hex)
    # read all messages in queue until one is correct or get cancelled
    while True:
        msg = await queue_r.receive()
        if msg.command == b"r":
            splitdata = msg.data.split(b",")
            try:
                rec_regnum, rec_value = splitdata
            except ValueError:
                raise BadDataError(
                    f"read response data, {msg!r}, can't be split into a reg num and reg val (no comma!)"
                )
            else:
                if int(rec_regnum, 16) == int(regnum_hex, 16):
                    return rec_value


async def write_register(
    fout,
    queue_w: trio.abc.ReceiveChannel,
    regnum: int,
    content: bytes,
    registerBitWidth: int,
):
    """
        write register on device

        queue_w is a read queue where "w" messages are deposited, i.e. a trio.abc.ReceiveChannel

        Assumes reciver_loop is running in another task

        regnum: an integer register number

        content: bytes to write to the regnum or an integer.
            The integer is converted to little-endian bytes,
            and negative integers aren't allowed.

        returns None

        raises exception if not success
        """
    asciiSerialComVersion = b"00"
    appVersion = b"00"
    regnum_hex = check_register_number(regnum)
    content_hex = check_register_content(content, registerBitWidth)
    data = regnum_hex + b"," + content_hex
    send_message(fout, asciiSerialComVersion, appVersion, b"w", data)
    await send_message(fout, asciiSerialComVersion, appVersion, b"w", data)
    # read all messages in queue until one is correct or get cancelled
    while True:
        msg = await queue_w.receive()
        if msg.command == b"w":
            try:
                msg_regnum = int(msg.data, 16)
            except ValueError:
                raise BadDataError(
                    f"write response data, {msg.data!r}, isn't a valid register number"
                )
            else:
                if msg_regnum == int(regnum_hex, 16):
                    return


async def send_message(
    fout, asciiSerialComVersion: bytes, appVersion: bytes, command: bytes, data: bytes
) -> None:
    """
    Low-level message send command
    Does not check if command is defined command

    command: single byte lower-case letter command/message type
    data: bytes or None

    returns None
    """
    msg = ASC_Message(asciiSerialComVersion, appVersion, command, data)
    message = msg.get_packed()
    print(
        "send_message: command: {!r} data: {!r} message: {!r}".format(
            command, data, message
        )
    )
    await fout.write(message)
    await fout.flush()


async def receiver_loop(
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
        msg = await receive_message(fin, buf, asciiSerialComVersion, appVersion)
        if msg:
            if msg.command == b"w" and queue_w:
                await queue_w.send(msg)
            elif msg.command == b"r" and queue_r:
                await queue_r.send(msg)
            elif msg.command == b"s" and queue_s:
                await queue_s.send(msg)
            else:
                pass


async def receive_message(
    fin, buf: Circular_Buffer_Bytes, asciiSerialComVersion: bytes, appVersion: bytes
) -> ASC_Message:
    """
    You usually won't need this, instead use receiver_loop

    fin: file-like object to read from

    uses Circular_Buffer_Bytes to keep track of input within and between invocations of receive_message

    if no frame is received, all members ASC_Message will be None

    """
    frame = await frame_from_stream(fin, buf)
    if frame is None:
        return ASC_Message(None, None, None, None)
    print("received message: {}".format(frame))
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
