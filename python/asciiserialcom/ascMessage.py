"""
ASCII Serial Com Message Class
"""

import crcmod
from .ascErrors import *


class ASC_Message(object):
    """
    Struct-type class to hold a message

    The user should access members:

    ascVersion
    appVersion
    command
    data
    """

    crcFunc = crcmod.predefined.mkPredefinedCrcFun("crc-16-dnp")

    def __init__(self, ascVersion, appVersion, command, data):
        if ascVersion is None or appVersion is None or command is None or data is None:
            self.ascVersion = None
            self.appVersion = None
            self.command = None
            self.data = None
        else:
            self.ascVersion = bytes(ascVersion)
            self.appVersion = bytes(appVersion)
            self.command = self._check_command(command)
            self.data = self._check_data(command, data)
            assert len(self.ascVersion) == 1
            assert len(self.appVersion) == 1

    def get_packed(self):
        """
        Packs command and data into a frame with checksum

        command: length 1 bytes

        data: data as bytes

        returns data frame as bytes
        """
        if not self:
            return None
        message = b">%c%c%c%b." % (
            self.ascVersion,
            self.appVersion,
            self.command,
            self.data,
        )
        checksum = self.compute_checksum(message)
        message += checksum + b"\n"
        return message

    @staticmethod
    def unpack(frame):
        """
        Unpacks a data frame into a ASC_Message while verifying checksum

        frame: bytes or bytearray

        returns a new ASC_Message
        """
        original_frame = frame
        comp_checksum = ASC_Message.compute_checksum(frame)
        frame, checksum = frame.split(b".")
        checksum = checksum.rstrip(b"\n")
        if checksum != comp_checksum:
            raise MessageIntegrityError("Message checksums don't match")
        #            self.nCrcErrors += 1
        #            if crcFailBehavior != "pass":
        #                print(
        #                    "Checksum mismatch, computed: {} received: {}".format(
        #                        comp_checksum, checksum
        #                    )
        #                )
        #                if crcFailBehavior == "throw":
        #                    raise MessageIntegrityError("Message checksums don't match")
        #                elif crcFailBehavior == "warn":
        #                    pass
        #                else:
        #                    raise ConfigurationError(
        #                        "crcFailBehavior value not understood: ", crcFailBehavior
        #                    )
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
            result = ASC_Message(ascVersion, appVersion, command, data)
            return result

    @staticmethod
    def compute_checksum(frame):
        """
        computes the checksum of the given message
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
        result = "{:04X}".format(ASC_Message.crcFunc(frame)).encode("ascii")
        # print("checksum computed to be: ",result)
        return result

    @staticmethod
    def _check_command(command):
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

    @staticmethod
    def _check_data(command, data):
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

    def __bool__(self):
        if self.command is None:
            return False
        return True

    def __eq__(self, other):
        return (
            self.ascVersion == other.ascVersion
            and self.appVersion == other.appVersion
            and self.command == other.command
            and self.data == other.data
        )

    def __str__(self):
        result = "ASC_Message: ascVersion: {0.ascVersion:}, appVersion: {0.appVersion}, command: {0.command:}, data: {0.data}".format(
            self
        )
        return result
