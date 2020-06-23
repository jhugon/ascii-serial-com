"""
ASCII Serial Com Python Interface
"""

import math
import datetime
import io
import crcmod


class Circular_Buffer_Bytes(object):
    """
    Implements a circular buffer as a bytearray object
    """

    def __init__(self, N):
        self.capacity = N
        self.data = bytearray(N)
        self.iStart = 0
        self.iStop = 0
        self.size = 0

    def push_back(self, b):
        """
        Add bytes to the end of the circular buffer

        Does so by overwriting earlier contents if necessary
        """
        for x in b:
            self.data[self.iStop] = x
            self.iStop = (self.iStop + 1) % self.capacity
            if self.size == self.capacity:
                self.iStart = (self.iStart + 1) % self.capacity
            else:
                self.size += 1

    def push_front(self, b):
        """
        Add bytes to the start of the circular buffer

        Does so by overwriting later contents if necessary
        """
        for x in reversed(b):
            self.iStart = (self.iStart - 1) % self.capacity
            self.data[self.iStart] = x
            if self.size == self.capacity:
                self.iStop = (self.iStop - 1) % self.capacity
            else:
                self.size += 1

    def pop_front(self, N):
        """
        Pop the first N bytes off of start of the circular buffer and return them
        """
        if N > self.capacity:
            raise ValueError(
                "N is greater than capacity of buffer: ", N, " > ", self.capacity
            )
        N = min(N, self.size)
        result = bytearray(N)
        for i in range(min(N, self.size)):
            result[i] = self.data[self.iStart]
            self.iStart = (self.iStart + 1) & self.capacity
            self.size -= 1
        return result

    def pop_back(self, N):
        """
        Pop the last N bytes off of the end of the circular buffer and return them
        """
        if N > self.capacity:
            raise ValueError(
                "N is greater than capacity of buffer: ", N, " > ", self.capacity
            )
        N = min(N, self.size)
        result = bytearray(N)
        for i in range(N):
            j = (self.iStop - N) % self.capacity
            result[i] = self.data[j]
        self.iStop = (self.iStop - N) % self.capacity
        self.size -= N
        return result

    def removeFrontTo(self, val, inclusive=False):
        """
        Remove front elements up to given val

        if inclusive, then remove the given val, otherwise all before the given val

        returns None
        """
        while True:
            if self.isEmpty():
                return
            elif self.data[self.iStart] == val:
                if inclusive:
                    self.iStart = (self.iStart + 1) % self.capacity
                    self.size -= 1
                return
            else:
                self.iStart = (self.iStart + 1) % self.capacity
                self.size -= 1

    def removeBackTo(self, val, inclusive=False):
        """
        Remove back elements to given val

        if inclusive, then remove the given val, otherwise all after the given val

        returns None
        """
        while True:
            if self.isEmpty():
                return
            elif self.data[self.iStop] == val:
                if inclusive:
                    self.iStop = (self.iStop - 1) % self.capacity
                    self.size -= 1
                return
            else:
                self.iStop = (self.iStop - 1) % self.capacity
                self.size -= 1

    def count(self, x):
        """
        Returns number of elements equal to x in buffer
        """
        result = 0
        for i in range(self.size):
            j = (self.iStart + i) % self.capacity
            if self.data[j] == x:
                result += 1
        return result

    def findFirst(self, x):
        """
        Returns the index (from iStart) of the first occurance of x

        Returns None if no x found
        """
        for i in range(self.size):
            j = (self.iStart + i) % self.capacity
            if self.data[j] == x:
                return i
        return None

    def __len__(self):
        return self.size

    def isFull(self):
        return len(self) == self.capacity

    def isEmpty(self):
        return len(self) == 0


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
        self.send_message(b"r", regnum_hex, self.f)
        timeout_time = datetime.now() + datetime.timedelta(seconds=timeout)
        while datetime.now() < timeout_time:
            _, _, command, data = self.receive_message(self.f)
            if command is None:
                continue
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
            _, _, rec_command, rec_data = self.receive_message(self.f)
            if rec_command is None:
                continue
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
        if issubclass(f, io.TextIOBase):
            raise TypeError(
                "File objects passed to Ascii_Serial_Com should be opened in binary not text mode"
            )
        message = self._pack_message(command, data)
        f.write(message)
        f.flush()

    def receive_message(self, f, timeout=None):
        """
        f: file-like object to write the message to

        timeout: time to wait for a reply in seconds. defaults to 100 ms

        returns (ascVersion, appVersion, command, data) all as bytes
            ascVersion is the ASCII-Serial-Com format version
            appVersion is a user supplied application version

        if no frame is received, all members of return tuple will be None

        """
        frame = self._frame_from_stream(f, timeout=timeout)
        if frame is None:
            return None, None, None, None
        ascVersion, appVersion, command, data = self._unpack_message(frame)
        if ascVersion != self.asciiSerialComVersion and self.ascVersionMismatchThrow:
            raise Exception(
                "Message ASCII-Serial-Com version mismatch. Message version: {} Expected version: {}".format(
                    ascVersion, self.asciiSerialComVersion
                )
            )
        if appVersion != self.appVersion and self.appVersionMismatchThrow:
            raise Exception(
                "Message ASCII-Serial-Com application version mismatch. Message version: {} Expected version: {}".format(
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
                    raise ValueError("Message checksums don't match")
                elif self.crcFailBehavior == "warn":
                    pass
                else:
                    raise ValueError(
                        "crcFailBehavior value not understood: ", self.crcFailBehavior
                    )
        frame = frame.lstrip(b">")
        try:
            ascVersion = frame[0]
            appVersion = frame[1]
            command = frame[3]
            data = frame[4, -1]
        except IndexError:
            raise Exception("Malformed frame: ", original_frame)
        else:
            return ascVersion, appVersion, command, data

    def _frame_from_stream(self, f, timeout):
        """
        Reads bytes from f and attempts to identify a message frame.

        f: file-like object

        timeout: float in seconds to wait for bytes on stream

        returns: frame as bytes; None if no frame found in stream
        """
        if issubclass(f, io.TextIOBase):
            raise TypeError(
                "File objects passed to Ascii_Serial_Com should be opened in binary not text mode"
            )
        timeout_time = datetime.now() + datetime.timedelta(seconds=timeout)
        while datetime.now() < timeout_time:
            b = f.read(16)
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

        ## since max frame length is 64, and other parts of frame are 8 bytes
        ## data must be length <= 56
        MAXDATALEN = 56
        if len(data) > MAXDATALEN:
            raise ValueError("Data can only be <= len", MAXDATALEN, "is", len(data))

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
        if frame[0] != b">" or frame[-1] != b"\n":
            raise Exception(
                "Inproperly formatted frame: incorrect start and end chars: ", frame
            )
        if frame.count(b".") != 1:
            raise Exception(
                "Inproperly formatted frame: no end of data character '.': ", frame
            )
        frame = frame.split(b".")[0] + b"."
        result = hex(self.crcFunc(frame)).upper()
        return result

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
    f = None
    asc = Ascii_Serial_Com(f, 32)
    print(asc)
