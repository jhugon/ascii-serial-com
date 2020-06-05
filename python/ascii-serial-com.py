"""
ASCII Serial Com Python Interface
"""

import math


class Ascii_Serial_Com(object):
    """
    ASCII Serial Com Python Interface Class

    This interface is assumed to be a host, not a device
    """

    def __init__(
        self,
        registerBitWidth,
        crcFailBehavior="throw",
        appVersion=u"0",
        asciiSerialComVersion=u"0",
    ):
        self.registerBitWidth = registerBitWidth
        self.registerByteWidth = int(math.ceil(registerBitWidth / 8))
        self.crcFailBehavior = crcFailBehavior
        self.appVersion = appVersion
        self.asciiSerialComVersion = asciiSerialComVersion

    def read_register(self, regnum):
        """
        Read register on device

        regnum: an integer register number

        returns register content as bytes
        """
        pass

    def write_register(self, regnum, content):
        """
        write register on device

        regnum: an integer register number

        content: bytes to write to the regnum or an integer.
            The integer is converted to little-endian bytes, 
            and negative integers aren't allowed.

        returns register content as bytes
        """
        pass

    def send_message(self, command, data, f):
        """
        Low-level message send command
        Does not check if command is defined command

        command: single byte lower-case letter command/message type
        data: bytes or None
        f: file-like object to write the message to

        returns None
        """
        pass

    def receive_message(self, f):
        """
        f: file-like object to write the message to

        returns (command, data)

        """
        pass

    def __str__(self):
        pass

    def _check_command(self, command):
        """
        Checks command meets format specification

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

    def _check_register_content(self, content):
        """
        Checks register content matches format specification and register width

        returns properly content

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
            b"{:0X}" % content
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
        return content.lower()
