"""
ASCII Serial Com Python Interface
"""

import math
import datetime
import time
import io
import queue
import threading
import traceback
import sys
from .ascErrors import *
from .circularBuffer import Circular_Buffer_Bytes
from .ascMessage import ASC_Message


def dummy_s_consumer(q, event):
    while True:
        try:
            q.get(timeout=0.01)
        except queue.Empty:
            pass
        if event.is_set():
            return


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
        receiver_queue_s_consumer=None,
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
        receiver_queueu_s_consumer: a callable that takes
            a Queue object and threading.Event as
            arguments and deals with 's' messages. Will be
            ran in another thread. Should periodically
            check if the event.is_set(), and if true,
            return.
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
        self.receiver_queue_s_consumer = receiver_queue_s_consumer
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

        self.receiver_thread = threading.Thread(target=self.receiver_loop, daemon=True)
        self.receiver_queue_w = queue.Queue()
        self.receiver_queue_r = queue.Queue()
        self.receiver_queue_s = queue.Queue()
        self.receiver_thread_del_event = threading.Event()

        if self.receiver_queue_s_consumer is None:
            self.receiver_queue_s_consumer = dummy_s_consumer
        self.receiver_queue_s_consumer_thread_del_event = threading.Event()
        self.receiver_queue_s_consumer_thread = threading.Thread(
            target=self.receiver_queue_s_consumer,
            args=(
                self.receiver_queue_s,
                self.receiver_queue_s_consumer_thread_del_event,
            ),
            daemon=True,
        )

        self.receiver_queue_s_consumer_thread.start()
        self.receiver_thread.start()

    def __del__(self):
        self.receiver_queue_s_consumer_thread_del_event.set()
        self.receiver_thread_del_event.set()
        self.receiver_queue_s_consumer_thread.join(0.5)
        self.receiver_thread.join(0.5)

    def read_register(self, regnum, timeout=1):
        """
        Read register on device

        regnum: an integer register number from 0 to 0xFFFF

        timeout: time to wait for a reply in seconds. defaults to 1

        returns register content as bytes
        """

        regnum_hex = self._check_register_number(regnum)
        self.send_message(b"r", regnum_hex)
        try:
            msg = self.receiver_queue_r.get(timeout=timeout)
        except queue.Empty:
            raise ResponseTimeoutError(
                f"Timout while waiting for response to read {regnum} timeout={timeout} s"
            )
        else:
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
        # get here because got a non 'r' or wrong regnum message
        # read all messages in queue until one is correct or queue is empty
        while True:
            try:
                msg = self.receiver_queue_r.get_nowait()
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
            except queue.Empty:
                raise ResponseTimeoutError(
                    f"Timout while waiting for response to read {regnum} timeout={timeout} s"
                )

    def write_register(self, regnum, content, timeout=1):
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
        regnum_hex = self._check_register_number(regnum)
        content_hex = self._check_register_content(content)
        data = regnum_hex + b"," + content_hex
        self.send_message(b"w", data)
        try:
            msg = self.receiver_queue_w.get(timeout=timeout)
        except queue.Empty:
            raise ResponseTimeoutError(
                f"Timout while waiting for response to write {regnum} {content} timeout={timeout} s"
            )
        else:
            if msg.command == b"w" and int(msg.data, 16) == int(regnum_hex, 16):
                return
            else:
                # read all messages in queue until one is correct or queue is empty
                while True:
                    try:
                        msg = self.receiver_queue_w.get_nowait()
                        if msg.command == b"w" and int(msg.data, 16) == int(
                            regnum_hex, 16
                        ):
                            return
                    except queue.Empty:
                        raise ResponseTimeoutError(
                            f"Timout while waiting for response to write {regnum} {content} timeout={timeout} s"
                        )

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

    def receiver_loop(self):
        while True:
            try:
                msg = self._receive_message(0.05)
            except Exception as e:
                traceback.print_exception(type(e), e, sys.last_traceback)
            else:
                if msg:
                    if msg.command == b"w":
                        self.receiver_queue_w.put(msg)
                    elif msg.command == b"r":
                        self.receiver_queue_r.put(msg)
                    elif msg.command == b"s":
                        self.receiver_queue_s.put(msg)
                    else:
                        pass
                    # print(f" len of r queue: {self.receiver_queue_r.qsize()}")
            if self.receiver_thread_del_event.is_set():
                return

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

    def _receive_message(self, timeout):
        """
        returns a ASC_Message

        if no frame is received, all members ASC_Message will be None

        """
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

    def _frame_from_stream(self, timeout):
        """
        Reads bytes from file-like object and attempts to identify a message frame.

        returns: frame as bytes; None if no frame found in stream
        """
        timeout_time = datetime.datetime.now() + datetime.timedelta(seconds=timeout)
        while datetime.datetime.now() < timeout_time:
            b = self.fin.read()
            self.buffer.push_back(b)
            self.buffer.removeFrontTo(b">", inclusive=False)
            if len(self.buffer) == 0:
                continue
            iNewline = self.buffer.findFirst(b"\n")
            if iNewline is None:
                continue
            return self.buffer.pop_front(iNewline + 1)
        return None

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
