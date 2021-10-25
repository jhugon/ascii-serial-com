"""
ASCII Serial Com Python Interface
"""

import math
from .ascErrors import *
from .asciiSerialComLowLevel import send_message
import trio


async def example_read_reg(fin, fout, regnum):
    """
    Example of using the code in this module

    Would probably want to wrap this in a cancel timeout:

        with trio.move_on_after(5):

    or just run it directly like:

        trio.run(example_read_reg,trio.open_file("xxx",mode="br",buffering=0),trio.open_file("xxx",mode="bw",buffering=0),0)
    """

    result = None
    with trio.move_on_after(
        1e6
    ) as cancel_scope:  # arbitrarily large; intend parent to actually set timeout
        async with trio.open_nursery() as nursery:
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


async def read_register(fout, queue_r, regnum):
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
                    f"read response data, {data!r}, can't be split into a reg num and reg val (no comma!)"
                )
            else:
                if int(rec_regnum, 16) == int(regnum_hex, 16):
                    return rec_value


async def write_register(fout, queue_w, regnum, content, registerBitWidth):
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
    send_message(b"w", data)
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


def check_register_number(num):
    """
        Checks register number passed to read_register/write_register matches format specification

        returns properly formatted content

        raises BadRegisterNumberError if not fomatted correctly or incorrect bit width
        """
    if isinstance(num, int):
        if num < 0:
            raise BadRegisterNumberError(f"register number, {num}, must be positive")
        if num.bit_length() > 16:
            raise BadRegisterNumberError(
                f"register number {num} = 0x{num:X} requires {num.bit_length()} bits which is > 16"
            )
        num = b"%04X" % num
    if isinstance(num, str):
        num = num.encode("ascii")
    if not (isinstance(num, bytes) or isinstance(num, bytearray)):
        raise BadRegisterNumberError(
            f"register number {num} isn't bytes or bytearray type or int is {type(num)}"
        )
    if len(num) < 4:
        num = b"0" * (4 - len(num)) + num
    if len(num) != 4:
        raise BadRegisterNumberError(
            f"register number {num} should be 4 bytes, but is {len(num)} bytes"
        )
    if not num.isalnum():
        raise BadRegisterNumberError(
            f"register number must be ASCII letters and numbers not: {num}"
        )
    try:
        int(num, 16)
    except:
        raise BadRegisterNumberError(
            f"register number, {num},must be convertible to hexadecimal number"
        )
    if int(num, 16).bit_length() > 16:  # in case num is bytes so missed earlier check
        raise BadRegisterNumberError(
            f"register number, {num}, requires more than 16 bits"
        )
    return num.upper()


def check_register_content(content, registerBitWidth):
    """
        Checks register content passed to write_register matches format specification and register width

        returns properly formatted content

        raises BadRegisterContentError if not fomatted correctly or incorrect bit width
        """
    registerByteWidth = int(math.ceil(registerBitWidth / 8))
    if isinstance(content, int):
        if content < 0:
            raise BadRegisterContentError(
                "content argument", content, "must be positive"
            )
        if content.bit_length() > registerBitWidth:
            raise BadRegisterContentError(
                f"content argument {content} = 0x{content:X} requires {content.bit_length()} bits which is > registerBitWidth = {registerBitWidth}"
            )
        content = b"%0X" % content
    if isinstance(content, str):
        content = content.encode("ascii")
    if not (isinstance(content, bytes) or isinstance(content, bytearray)):
        raise BadRegisterContentError(
            "content argument", content, "isn't bytes or bytearray type or int"
        )
    if len(content) < registerByteWidth * 2:
        content = b"0" * (registerByteWidth * 2 - len(content)) + content
    if len(content) != registerByteWidth * 2:
        raise BadRegisterContentError(
            "content argument ",
            content,
            "should be len ",
            registerByteWidth * 2,
            ", is len ",
            len(content),
        )
    if not content.isalnum():
        raise BadRegisterContentError(
            "content argument must be ASCII letters and numbers not: ", content
        )
    try:
        int(content, 16)
    except:
        raise BadRegisterContentError(
            "content argument must be convertible to hexadecimal number"
        )
    if (
        int(content, 16).bit_length() > registerBitWidth
    ):  # in case content is bytes so missed earlier check
        raise BadRegisterContentError(
            "content argument", content, "requires more bits than registerBitWidth"
        )
    return content.upper()


def convert_to_hex(num, N=2):
    """
    Converts integer to hexadecimal number as bytes

    num: integer. If str, bytes, or bytearray just converts to bytes
    N: (optional) amount to zero pad (but will use more if necessary). Default: 2

    returns bytes
    """

    result = None
    if isinstance(num, bytearray):
        result = bytes(num)
    elif isinstance(num, str):
        result = num.encode("ascii")
    elif isinstance(num, bytes):
        result = num
    else:
        formatstr = b"%0" + str(N).encode("ascii") + b"X"
        result = formatstr % (num)
    result = result.upper()
    if len(result) == 0:
        raise ValueError("num is a zero length bytes, not valid hex")
    lendiff = N - len(result)
    if lendiff > 0:
        result = b"0" * lendiff + result
    return result


def convert_from_hex(num):
    if isinstance(num, int):
        return result
    else:
        if len(num) == 0:
            raise ValueError("num is a zero length bytes, can't convert to int")
        return int(num, 16)
