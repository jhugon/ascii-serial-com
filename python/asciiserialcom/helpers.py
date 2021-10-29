from __future__ import annotations
import math
import logging

from .errors import *
from .circularBuffer import Circular_Buffer_Bytes

from typing import Union, Optional, Any
from collections.abc import Sequence


async def frame_from_stream(fin, buf: Circular_Buffer_Bytes) -> Optional[Sequence]:
    """
    Reads bytes from file-like object and attempts to identify a message frame. Uses circular_buffer buf.

    returns: frame as bytes; None if no frame found in stream
    """
    try:
        # logging.debug("about to read from fin")
        b = await fin.read()
    except ValueError:
        raise FileReadError
    except IOError:
        raise FileReadError
    else:
        if len(b) > 0:
            logging.debug(f"got {len(b)} bytes from fin")
        buf.push_back(b)
        buf.removeFrontTo(b">", inclusive=False)
        if len(buf) == 0:
            return None
        iNewline = buf.findFirst(b"\n")
        if iNewline is None:
            return None
        logging.debug("have a whole message")
        return buf.pop_front(iNewline + 1)


def check_register_number(num: Union[int, str, bytes, bytearray]) -> bytes:
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
            f"register number {num!r} should be 4 bytes, but is {len(num)} bytes"
        )
    if not num.isalnum():
        raise BadRegisterNumberError(
            f"register number must be ASCII letters and numbers not: {num!r}"
        )
    try:
        int(num, 16)
    except:
        raise BadRegisterNumberError(
            f"register number, {num!r},must be convertible to hexadecimal number"
        )
    if int(num, 16).bit_length() > 16:  # in case num is bytes so missed earlier check
        raise BadRegisterNumberError(
            f"register number, {num!r}, requires more than 16 bits"
        )
    return num.upper()


def check_register_content(
    content: Union[int, str, bytes, bytearray], registerBitWidth: int
) -> bytes:
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


def convert_to_hex(num: Union[bytes, bytearray, str, int], N: int = 2) -> bytes:
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


def convert_from_hex(num: Union[bytes, bytearray, str, int]) -> int:
    if isinstance(num, int):
        return num
    else:
        if len(num) == 0:
            raise ValueError("num is a zero length bytes, can't convert to int")
        return int(num, 16)
