"""
ASCII Serial Com Python Interface
"""

import math
import datetime
import time
import io
import selectors
from .ascErrors import *
from .circularBuffer import Circular_Buffer_Bytes
from .ascMessage import ASC_Message


class Ascii_Serial_Com(object):
    """
    ASCII Serial Com Python Interface Class

    This interface is assumed to be a host, not a device
    """

    def __init__(
        self,
        fin,
        fout,
        registerBitWidth,
        crcFailBehavior="throw",
        appVersion=b"0",
        asciiSerialComVersion=b"0",
        ascVersionMismatchThrow=True,
        appVersionMismatchThrow=False,
        sleepIfNothingReadTime=0.1,
        printMessages=False,
    ):
        """
        fin: binary file object streaming from the device
        fout: binary file object streaming to the device
        registerBitWidth: an int, probably 8 or 32
        crcFailBehavior: a str: "throw", "warn", "pass"
        appVersion: a len 1 byte
        asciiSerialComVersion: a len 1 byte
        ascVersionMismatchThrow: bool, if True, throws an
            error if message asciiSerialComVersion doesn't
            match one given to __init__
        appVersionMismatchThrow: bool, if True, throws an
            error if message appVersion doesn't
            match one given to __init__
        sleepIfNothingReadTime: float in seconds, how long
            to sleep before trying to read from stream
            again
        """
        if isinstance(fin, io.TextIOBase):
            raise TextFileNotAllowedError(
                f"fin, {fin},file object passed to Ascii_Serial_Com should be opened in binary not text mode"
            )
        if isinstance(fout, io.TextIOBase):
            raise TextFileNotAllowedError(
                f"fout, {fout},file object passed to Ascii_Serial_Com should be opened in binary not text mode"
            )
        self.fin = fin
        self.fout = fout
        self.registerBitWidth = registerBitWidth
        self.registerByteWidth = int(math.ceil(registerBitWidth / 8))
        self.crcFailBehavior = crcFailBehavior
        self.appVersion = appVersion
        self.asciiSerialComVersion = asciiSerialComVersion
        self.ascVersionMismatchThrow = ascVersionMismatchThrow
        self.appVersionMismatchThrow = appVersionMismatchThrow
        self.sleepIfNothingReadTime = sleepIfNothingReadTime
        self.printMessages = printMessages
        self.nBytesT = 0
        self.nBytesR = 0
        self.nCrcErrors = 0

        self.buffer = Circular_Buffer_Bytes(128)

        ### Receiver Thread

        self.selectorIn = selectors.DefaultSelector()
        self.selectorIn.register(self.fin, selectors.EVENT_READ)

    def read_register(self, regnum, timeout=None):
        """
        Read register on device

        regnum: an integer register number from 0 to 0xFFFF

        timeout: time to wait for a reply in seconds. defaults to 1

        returns register content as bytes
        """
        if timeout is None:
            timeout = 1.0

        regnum_hex = self._check_register_number(regnum)
        self.send_message(b"r", regnum_hex)
        timeout_time = datetime.datetime.now() + datetime.timedelta(seconds=timeout)
        while datetime.datetime.now() < timeout_time:
            msg = self.receive_message()
            if not msg:
                continue
            if msg.command == b"r":
                splitdata = msg.data.split(b",")
                try:
                    rec_regnum, rec_value = splitdata
                except ValueError:
                    raise BadDataError(
                        f"read response data, {data!r}, can't be split into a reg num and reg val (no comma!)"
                    )
                if int(rec_regnum, 16) == int(regnum_hex, 16):
                    return rec_value
        raise ResponseTimeoutError("Timout while waiting for response")

    def write_register(self, regnum, content, timeout=None):
        """
        write register on device

        regnum: an integer register number

        content: bytes to write to the regnum or an integer.
            The integer is converted to little-endian bytes,
            and negative integers aren't allowed.

        timeout: time to wait for a reply in seconds. defaults to 1

        returns None

        raises exception if not success
        """
        if timeout is None:
            timeout = 1.0
        regnum_hex = self._check_register_number(regnum)
        content_hex = self._check_register_content(content)
        data = regnum_hex + b"," + content_hex
        self.send_message(b"w", data)
        timeout_time = datetime.datetime.now() + datetime.timedelta(seconds=timeout)
        while datetime.datetime.now() < timeout_time:
            msg = self.receive_message()
            if not msg.command:
                continue
            if msg.command == b"w":
                if int(msg.data, 16) == int(regnum_hex, 16):
                    return
        raise ResponseTimeoutError("Timout while waiting for response")

    def send_message(self, command, data):
        """
        Low-level message send command
        Does not check if command is defined command

        command: single byte lower-case letter command/message type
        data: bytes or None

        returns None
        """
        msg = ASC_Message(self.asciiSerialComVersion, self.appVersion, command, data)
        message = msg.get_packed()
        if self.printMessages:
            print(
                "send_message: command: {} data: {} message: {}".format(
                    command, data, message
                )
            )
        self.fout.write(message)
        self.fout.flush()

    def receive_message(self, timeout=None):
        """
        timeout: (optional) time to wait for a reply in seconds. defaults to 100 ms

        returns a ASC_Message

        if no frame is received, all members ASC_Message will be None

        """
        if timeout is None:
            timeout = 0.1
        frame = self._frame_from_stream(timeout)
        if frame is None:
            return ASC_Message(None, None, None, None)
        if self.printMessages:
            print("received message: {}".format(frame))
        msg = ASC_Message.unpack(frame)
        if (
            msg.ascVersion != self.asciiSerialComVersion
            and self.ascVersionMismatchThrow
        ):
            raise AsciiSerialComVersionMismatchError(
                "Message version: {} Expected version: {}".format(
                    msg.ascVersion, self.asciiSerialComVersion
                )
            )
        if msg.appVersion != self.appVersion and self.appVersionMismatchThrow:
            raise ApplicationVersionMismatchError(
                "Message version: {} Expected version: {}".format(
                    msg.appVersion, self.appVersion
                )
            )
        return msg

    def getRegisterBitWidth(self):
        return self.registerBitWidth

    def getRegisterByteWidth(self):
        return self.registerByteWidth

    def __str__(self):
        result = """Ascii_Serial_Com Object:
  Register width: {0.registerBitWidth:} bits, {0.registerByteWidth} bytes
  N CRC Errors: {0.nCrcErrors:6}, CRC fail behavior: {0.crcFailBehavior}
  N Bytes transmit: {0.nBytesT:8}, receive: {0.nBytesR:8}""".format(
            self
        )
        return result

    def _frame_from_stream(self, timeout):
        """
        Reads bytes from file-like object and attempts to identify a message frame.

        timeout: float in seconds to wait for bytes on stream

        returns: frame as bytes; None if no frame found in stream
        """
        timeout_time = datetime.datetime.now() + datetime.timedelta(seconds=timeout)
        while datetime.datetime.now() < timeout_time:
            readyfiles = self.selectorIn.select(self.sleepIfNothingReadTime)
            if len(readyfiles) > 0:
                b = self.fin.read(16)
                if len(b) == 0:
                    time.sleep(self.sleepIfNothingReadTime)
                    continue
                else:
                    self.buffer.push_back(b)
                    self.buffer.removeFrontTo(b">", inclusive=False)
                    if len(self.buffer) == 0:
                        continue
                    iNewline = self.buffer.findFirst(b"\n")
                    if iNewline is None:
                        continue
                return self.buffer.pop_front(iNewline + 1)

    def _check_register_number(self, num):
        """
        Checks register number passed to read_register/write_register matches format specification

        returns properly formatted content

        raises BadRegisterNumberError if not fomatted correctly or incorrect bit width
        """
        if isinstance(num, int):
            if num < 0:
                raise BadRegisterNumberError(
                    f"register number, {num}, must be positive"
                )
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
        if (
            int(num, 16).bit_length() > 16
        ):  # in case num is bytes so missed earlier check
            raise BadRegisterNumberError(
                f"register number, {num}, requires more than 16 bits"
            )
        return num.upper()

    def _check_register_content(self, content):
        """
        Checks register content passed to write_register matches format specification and register width

        returns properly formatted content

        raises BadRegisterContentError if not fomatted correctly or incorrect bit width
        """
        if isinstance(content, int):
            if content < 0:
                raise BadRegisterContentError(
                    "content argument", content, "must be positive"
                )
            if content.bit_length() > self.registerBitWidth:
                raise BadRegisterContentError(
                    f"content argument {content} = 0x{content:X} requires {content.bit_length()} bits which is > registerBitWidth = {self.registerBitWidth}"
                )
            content = b"%0X" % content
        if isinstance(content, str):
            content = content.encode("ascii")
        if not (isinstance(content, bytes) or isinstance(content, bytearray)):
            raise BadRegisterContentError(
                "content argument", content, "isn't bytes or bytearray type or int"
            )
        if len(content) < self.registerByteWidth * 2:
            content = b"0" * (self.registerByteWidth * 2 - len(content)) + content
        if len(content) != self.registerByteWidth * 2:
            raise BadRegisterContentError(
                "content argument ",
                content,
                "should be len ",
                self.registerByteWidth * 2,
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
            int(content, 16).bit_length() > self.registerBitWidth
        ):  # in case content is bytes so missed earlier check
            raise BadRegisterContentError(
                "content argument", content, "requires more bits than registerBitWidth"
            )
        return content.upper()

    def _convert_to_hex(self, num, N=2):
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

    def _convert_from_hex(self, num):
        if isinstance(num, int):
            return result
        else:
            if len(num) == 0:
                raise ValueError("num is a zero length bytes, can't convert to int")
            return int(num, 16)


if __name__ == "__main__":
    f = None
    asc = Ascii_Serial_Com(f, 32)
    print(asc)
