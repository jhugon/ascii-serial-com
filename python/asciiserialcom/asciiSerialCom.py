"""
ASCII Serial Com Python Interface
"""

import math
import datetime
import io
import crcmod
from .ascErrors import *
from .circularBuffer import Circular_Buffer_Bytes


class Ascii_Serial_Com(object):
    """
    ASCII Serial Com Python Interface Class

    This interface is assumed to be a host, not a device
    """

    def __init__(
        self,
        f,
        registerBitWidth,
        crcFailBehavior="throw",
        appVersion=b"0",
        asciiSerialComVersion=b"0",
        ascVersionMismatchThrow=True,
        appVersionMismatchThrow=False,
    ):
        """
        f: binary file object used for reading and
            writing to device
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
        """
        if isinstance(f, io.TextIOBase):
            raise TextFileNotAllowedError(
                "File objects passed to Ascii_Serial_Com should be opened in binary not text mode"
            )
        self.f = f
        self.registerBitWidth = registerBitWidth
        self.registerByteWidth = int(math.ceil(registerBitWidth / 8))
        self.crcFailBehavior = crcFailBehavior
        self.appVersion = appVersion
        self.asciiSerialComVersion = asciiSerialComVersion
        self.ascVersionMismatchThrow = ascVersionMismatchThrow
        self.appVersionMismatchThrow = appVersionMismatchThrow
        self.nBytesT = 0
        self.nBytesR = 0
        self.nCrcErrors = 0

        self.buffer = Circular_Buffer_Bytes(128)
        self.crcFunc = crcmod.predefined.mkPredefinedCrcFun("crc-16-dnp")

    def read_register(self, regnum, timeout=None):
        """
        Read register on device

        regnum: an integer register number

        timeout: time to wait for a reply in seconds. defaults to 1

        returns register content as bytes
        """
        if timeout is None:
            timeout = 1.0
        regnum_hex = self._convert_to_hex(regnum)
        self.send_message(b"r", regnum_hex)
        timeout_time = datetime.datetime.now() + datetime.timedelta(seconds=timeout)
        while datetime.datetime.now() < timeout_time:
            _, _, command, data = self.receive_message()
            if command is None:
                continue
            if command == b"r":
                rec_regnum, rec_value = data.split(b",")
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
        regnum_hex = self._convert_to_hex(regnum)
        content_hex = self._check_register_content(content)
        data = regnum_hex + b"," + content_hex
        self.send_message(b"w", data, self.f)
        timeout_time = datetime.datetime.now() + datetime.timedelta(seconds=timeout)
        while datetime.datetime.now() < timeout_time:
            _, _, rec_command, rec_data = self.receive_message()
            if rec_command is None:
                continue
            if rec_command == b"w":
                if int(rec_data, 16) == int(regnum_hex, 16):
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
        message = self._pack_message(command, data)
        self.f.write(message)
        self.f.flush()

    def receive_message(self, timeout=None):
        """
        timeout: (optional) time to wait for a reply in seconds. defaults to 100 ms

        returns (ascVersion, appVersion, command, data) all as bytes
            ascVersion is the ASCII-Serial-Com format version
            appVersion is a user supplied application version

        if no frame is received, all members of return tuple will be None

        """
        if timeout is None:
            timeout = 0.1
        frame = self._frame_from_stream(timeout)
        if frame is None:
            return None, None, None, None
        ascVersion, appVersion, command, data = self._unpack_message(frame)
        if ascVersion != self.asciiSerialComVersion and self.ascVersionMismatchThrow:
            raise AsciiSerialComVersionMismatchError(
                "Message version: {} Expected version: {}".format(
                    ascVersion, self.asciiSerialComVersion
                )
            )
        if appVersion != self.appVersion and self.appVersionMismatchThrow:
            raise ApplicationVersionMismatchError(
                "Message version: {} Expected version: {}".format(
                    appVersion, self.appVersion
                )
            )
        return ascVersion, appVersion, command, data

    def __str__(self):
        result = """Ascii_Serial_Com Object:
  Register width: {0.registerBitWidth:} bits, {0.registerByteWidth} bytes
  N CRC Errors: {0.nCrcErrors:6}, CRC fail behavior: {0.crcFailBehavior}
  N Bytes transmit: {0.nBytesT:8}, receive: {0.nBytesR:8}""".format(
            self
        )
        return result

    def _pack_message(self, command, data):
        """
        Packs command and data into a frame with checksum

        command: length 1 bytes

        data: data as bytes

        returns data frame as bytes
        """
        command = self._check_command(command)
        data = self._check_data(command, data)
        message = b">%c%c%c%b." % (
            self.asciiSerialComVersion,
            self.appVersion,
            command,
            data,
        )
        checksum = self._compute_checksum(message)
        message += checksum + b"\n"
        return message

    def _unpack_message(self, frame):
        """
        Unpacks a data frame into command and data while verifying checksum

        frame: bytes or bytearray

        returns (ascVersion, appVersion, command, data) all as bytes
            ascVersion is the ASCII-Serial-Com format version
            appVersion is a user supplied application version
        """
        original_frame = frame
        comp_checksum = self._compute_checksum(frame)
        frame, checksum = frame.split(b".")
        checksum = checksum.rstrip(b"\n")
        if checksum != comp_checksum:
            self.nCrcErrors += 1
            if self.crcFailBehavior != "pass":
                print(
                    "Checksum mismatch, computed: {} received: {}".format(
                        comp_checksum, checksum
                    )
                )
                if self.crcFailBehavior == "throw":
                    raise MessageIntegrityError("Message checksums don't match")
                elif self.crcFailBehavior == "warn":
                    pass
                else:
                    raise ConfigurationError(
                        "crcFailBehavior value not understood: ", self.crcFailBehavior
                    )
        frame = frame.lstrip(b">")
        try:
            ascVersion = frame[0]
            appVersion = frame[1]
            command = frame[2]
            data = frame[3:]
        except IndexError:
            raise MalformedFrameError(original_frame)
        else:
            ascVersion = chr(ascVersion).encode("ascii")
            appVersion = chr(appVersion).encode("ascii")
            command = chr(command).encode("ascii")
            return ascVersion, appVersion, command, data

    def _frame_from_stream(self, timeout):
        """
        Reads bytes from file-like object and attempts to identify a message frame.

        timeout: float in seconds to wait for bytes on stream

        returns: frame as bytes; None if no frame found in stream
        """
        if isinstance(self.f, io.TextIOBase):
            raise TextFileNotAllowedError(
                "File objects passed to Ascii_Serial_Com should be opened in binary not text mode"
            )
        timeout_time = datetime.datetime.now() + datetime.timedelta(seconds=timeout)
        while datetime.datetime.now() < timeout_time:
            b = self.f.read(16)
            self.buffer.push_back(b)
            self.buffer.removeFrontTo(b">", inclusive=False)
            if len(self.buffer) == 0:
                continue
            iNewline = self.buffer.findFirst(b"\n")
            if iNewline is None:
                continue
            return self.buffer.pop_front(iNewline + 1)

    def _check_command(self, command):
        """
        Checks command meets format specification

        command: length 1 byte or bytearray

        returns properly formatted command byte

        raises BadCommandError if not fomatted correctly
        """
        if isinstance(command, str):
            command = command.encode("ascii")
        if isinstance(command, bytearray):
            command = bytes(command)
        if not isinstance(command, bytes):
            raise BadCommandError(
                "command argument", command, "isn't bytes, str, or bytearray type"
            )
        if len(command) != 1:
            raise BadCommandError(
                "command argument should be len 1, is len ", len(command)
            )
        if not command.isalpha():
            raise BadCommandError(
                "command argument must be an ASCII letter not: ", command
            )
        return command.lower()

    def _check_data(self, command, data):
        """
        Checks data payload meets format specification for given command

        command: length 1 byte or bytearray

        data: bytes, bytearray, or str data payload of message

        returns None

        raises BadDataError if not fomatted correctly
        """

        ## since max frame length is 64, and other parts of frame are 8 bytes
        ## data must be length <= 56
        if isinstance(data, str):
            data = data.encode("ascii")
        if isinstance(data, bytearray):
            data = bytes(data)
        if not isinstance(data, bytes):
            raise BadDataError("Data must be bytes or bytearray")
        MAXDATALEN = 56
        if len(data) > MAXDATALEN:
            raise BadDataError("Data can only be <= len", MAXDATALEN, "is", len(data))
        return data

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
                    "content argument",
                    content,
                    "requires more bits than registerBitWidth",
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
                self.registerByteWidth,
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

    def _compute_checksum(self, frame):
        """
        Computes the checksum for the given data frame from the `>' through the `.'

        frame: bytes representing the frame

        returns checksum as hexadecimal (capitals) bytes
        """
        if len(frame) == 0:
            raise MalformedFrameError("Zero length frame")
        if frame[0] != b">"[0] or ((frame[-1] != b"\n"[0]) and (frame[-1] != b"."[0])):
            raise MalformedFrameError("Incorrect start and/or end chars: ", frame)
        if frame.count(b".") != 1:
            raise MalformedFrameError(
                "Inproperly formatted frame: no end of data character '.': ", frame
            )
        frame = frame.split(b".")[0] + b"."
        result = hex(self.crcFunc(frame)).upper().encode("ascii")
        return result

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
