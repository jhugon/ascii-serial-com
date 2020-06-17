"""
ASCII Serial Com Python Interface
"""

import math
import datetime


class Ascii_Serial_Com(object):
    """
    ASCII Serial Com Python Interface Class

    This interface is assumed to be a host, not a device
    """

    def __init__(
        self,
        registerBitWidth,
        crcFailBehavior="throw",
        appVersion=b"0",
        asciiSerialComVersion=b"0",
    ):
        """
        registerBitWidth: an int, probably 8 or 32
        crcFailBehavior: a str: "throw", "warn", "pass"
        appVersion: a len 1 byte
        asciiSerialComVersion: a len 1 byte
        """
        self.registerBitWidth = registerBitWidth
        self.registerByteWidth = int(math.ceil(registerBitWidth / 8))
        self.crcFailBehavior = crcFailBehavior
        self.appVersion = appVersion
        self.asciiSerialComVersion = asciiSerialComVersion
        self.nBytesT = 0
        self.nBytesR = 0
        self.nCrcErrors = 0

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
        self.send_message(b"r", regnum_hex, self.f)
        timeout_time = datetime.now() + datetime.timedelta(seconds=timeout)
        while datetime.now() < timeout_time:
            command, data = self.receive_message(self.f)
            if command == b"r":
                rec_regnum, rec_value = data.split(b",")
                if int(rec_regnum, 16) == int(regnum_hex, 16):
                    return rec_value
        raise Exception("Timout while waiting for response")

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
        timeout_time = datetime.now() + datetime.timedelta(seconds=timeout)
        while datetime.now() < timeout_time:
            rec_command, rec_data = self.receive_message(self.f)
            if rec_command == b"w":
                if int(rec_data, 16) == int(regnum_hex, 16):
                    return
        raise Exception("Timout while waiting for response")

    def send_message(self, command, data, f):
        """
        Low-level message send command
        Does not check if command is defined command

        command: single byte lower-case letter command/message type
        data: bytes or None
        f: file-like object to write the message to

        returns None
        """
        message = self._pack_message(command, data)
        f.write(message)

    def receive_message(self, f):
        """
        f: file-like object to write the message to

        returns (command, data)

        """
        frame = self._frame_from_stream(f)
        command, data = self._unpack_message(frame)

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

    def _unpack_message(self, frame):
        """
        Unpacks a data frame into command and data while verifying checksum

        frame: bytes or bytearray

        returns (command, data) both as bytes
        """
        pass

    def _frame_from_stream(self, f):
        """
        Reads bytes from f and attempts to identify a message frame.

        f: file-like object

        returns: frame as bytes
        """
        pass

    def _check_command(self, command):
        """
        Checks command meets format specification

        command: length 1 byte or bytearray

        returns properly formatted command byte

        raises Exception if not fomatted correctly
        """
        if not (isinstance(command, bytes) or isinstance(command, bytearray)):
            raise Exception(
                "command argument", command, "isn't bytes or bytearray type"
            )
        if len(command) != 1:
            raise Exception("command argument should be len 1, is len ", len(command))
        if not command.isalpha():
            raise Exception("command argument must be an ASCII letter not: ", command)
        return command.lower()

    def _check_data(self, command, data):
        """
        Checks data payload meets format specification for given command

        command: length 1 byte or bytearray

        data: bytes or bytearray data payload of message

        returns None

        raises Exception if not fomatted correctly
        """
        pass
        # if command == b'w':

    def _check_register_content(self, content):
        """
        Checks register content passed to write_register matches format specification and register width

        returns properly formatted content

        raises Exception if not fomatted correctly or incorrect bit width
        """
        if isinstance(content, int):
            if content < 0:
                raise Exception("content argument", content, "must be positive")
            if content.bit_length() > self.registerBitWidth:
                raise Exception(
                    "content argument",
                    content,
                    "requires more bits than registerBitWidth",
                )
            content = b"{:0X}" % content
        if not (isinstance(content, bytes) or isinstance(content, bytearray)):
            raise Exception(
                "content argument", command, "isn't bytes or bytearray type or int"
            )
        if len(content) != self.registerByteWidth:
            raise Exception(
                "content argument should be len ",
                self.registerByteWidth,
                ", is len ",
                len(content),
            )
        if not content.isalnum():
            raise Exception(
                "content argument must be ASCII letters and numbers not: ", content
            )
        try:
            int(content, 16)
        except:
            raise Exception(
                "content argument must be convertible to hexadecimal number"
            )
        if (
            int(content, 16).bit_length() > self.registerBitWidth
        ):  # in case content is bytes so missed earlier check
            raise Exception(
                "content argument", content, "requires more bits than registerBitWidth"
            )
        return content.upper()

    def _compute_checksum(self, frame):
        """
        Computes the checksum for the given data frame from the `>' through the `.'

        frame: bytes representing the frame

        returns checksum as hexadecimal (capitals) bytes
        """
        pass

    def _convert_to_hex(self, num, N=2):
        if isinstance(content, bytes) or isinstance(content, bytearray):
            return num
        else:
            return b"{:0" + str(N) + "X}" % num

    def _convert_from_hex(self, num):
        if isinstance(int):
            return result
        else:
            return int(num, 16)


if __name__ == "__main__":
    asc = Ascii_Serial_Com(32)
    print(asc)
