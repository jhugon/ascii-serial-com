import unittest
import unittest.mock
from unittest.mock import patch
from asciiserialcom.asciiSerialCom import send_message, receive_message
from asciiserialcom.ascMessage import ASC_Message
from asciiserialcom.circularBuffer import Circular_Buffer_Bytes
from asciiserialcom.ascErrors import *
import crcmod
import datetime
import trio
import trio.testing


class MemoryWriteStream:
    def __init__(self, stream):
        self.stream = stream

    async def write(self, data):
        await self.stream.send_all(data)

    async def flush(self):
        return


class MemoryReadStream:
    def __init__(self, stream):
        self.stream = stream

    async def read(self):
        return await self.stream.receive_some()


class TestMessaging(unittest.TestCase):
    def setUp(self):
        self.crcFunc = crcmod.predefined.mkPredefinedCrcFun("crc-16-dnp")

    def test_send_message(self):
        async def run_test(args):
            send_stream, receive_stream = trio.testing.memory_stream_one_way_pair()
            write_stream = MemoryWriteStream(send_stream)
            await send_message(write_stream, b"0", b"0", *args)
            return await receive_stream.receive_some()

        for frame, args in [
            (b">00w.", (b"w", b"")),
            (b">00w0123456789ABCDEF.", (b"w", b"0123456789ABCDEF")),
            (b">00w" + b"A" * 56 + b".", (b"w", b"A" * 56)),
            (b">00zERrPhU10mfn.", (b"z", b"ERrPhU10mfn")),
        ]:
            with self.subTest(i="frame={}, args={}".format(frame, args)):
                written_data = trio.run(run_test, args)
                frame += "{:04X}".format(self.crcFunc(frame)).encode("ascii") + b"\n"
                self.assertEqual(written_data, frame)

    def test_receive_message(self):
        async def run_test(frame):
            send_stream, receive_stream = trio.testing.memory_stream_one_way_pair()
            read_stream = MemoryReadStream(receive_stream)
            buf = Circular_Buffer_Bytes(128)
            await send_stream.send_all(frame)
            return await receive_message(read_stream, buf, b"0", b"0")

        for frame, args in [
            (b">00w.", (b"w", b"")),
            (b">00w0123456789ABCDEF.", (b"w", b"0123456789ABCDEF")),
            (b">00w" + b"A" * 56 + b".", (b"w", b"A" * 56)),
            (b">00zERrPhU10mfn.", (b"z", b"ERrPhU10mfn")),
        ]:
            with self.subTest(i="frame={}, args={}".format(frame, args)):
                frame += "{:04X}".format(self.crcFunc(frame)).encode("ascii") + b"\n"
                msg = trio.run(run_test, frame)
                self.assertEqual(msg.command, args[0])
                self.assertEqual(msg.data, args[1])

    """
    def test_read_reg(self):
        fileMock = unittest.mock.MagicMock()
        asc = Ascii_Serial_Com(fileMock, fileMock, 32)
        read_array = []

        def read_func():
            if len(read_array) > 0:
                return read_array.pop(0)
            else:
                return b""

        fileMock.read.side_effect = read_func
        for reg_num, reg_val in [(2, 0x1234567A), (0xFF, 0)]:
            with self.subTest(i="reg_num={}, reg_val={}".format(reg_num, reg_val)):
                reply_message = b">00r%04X,%08X." % (reg_num, reg_val)
                reply_message += (
                    "{:04X}".format(self.crcFunc(reply_message)).encode("ascii") + b"\n"
                )
                read_array.append(reply_message)
                result = asc.read_register(reg_num)
                self.assertEqual(result, b"%08X" % reg_val)
                write_message = b">00r%04X." % (reg_num)
                write_message += (
                    "{:04X}".format(self.crcFunc(write_message)).encode("ascii") + b"\n"
                )
                fileMock.write.assert_called_once_with(write_message)
                fileMock.write.reset_mock()

                # reply with wrong reg number
                reply_message = b">00r%04X,%08X." % (reg_num + 1, reg_val)
                reply_message += (
                    "{:04X}".format(self.crcFunc(reply_message)).encode("ascii") + b"\n"
                )
                # fileMock.read.return_value = reply_message
                read_array.append(reply_message)
                with self.assertRaises(ResponseTimeoutError):
                    asc.read_register(reg_num, timeout=0.01)
                fileMock.write.reset_mock()

        # fileMock.read.return_value = b""
        with self.assertRaises(ResponseTimeoutError):
            asc.read_register(2, timeout=0.01)

    def test_write_reg(self):
        fileMock = unittest.mock.MagicMock()
        asc = Ascii_Serial_Com(fileMock, fileMock, 8)

        read_array = []

        def read_func():
            if len(read_array) > 0:
                return read_array.pop(0)
            else:
                return b""

        fileMock.read.side_effect = read_func
        for args, written, read in [
            ((b"0", b"00"), b">00w0000,00.", b">00w0000."),
            ((b"FF", b"E3"), b">00w00FF,E3.", b">00w00FF."),
            ((b"FFFF", b"E3"), b">00wFFFF,E3.", b">00wFFFF."),
            ((0, 0), b">00w0000,00.", b">00w0000."),
            ((0xFF, 0xE3), b">00w00FF,E3.", b">00w00FF."),
            ((0xFFFF, 0xE3), b">00wFFFF,E3.", b">00wFFFF."),
        ]:
            with self.subTest(
                i="args={}, written={}, read={}".format(args, written, read)
            ):
                written += (
                    "{:04X}".format(self.crcFunc(written)).encode("ascii") + b"\n"
                )
                read += "{:04X}".format(self.crcFunc(read)).encode("ascii") + b"\n"
                read_array.append(read)
                asc.write_register(*args)
                fileMock.write.assert_called_with(written)
                fileMock.write.reset_mock()

        for args, written in [
            ((b"0", b"00"), b">00w0000,00."),
            ((b"FF", b"E3"), b">00w00FF,E3."),
            ((b"FFFF", b"E3"), b">00wFFFF,E3."),
            ((0, 0), b">00w0000,00."),
            ((0xFF, 0xE3), b">00w00FF,E3."),
            ((0xFFFF, 0xE3), b">00wFFFF,E3."),
        ]:
            with self.subTest(i="args={}, written={}".format(args, written)):
                written += (
                    "{:04X}".format(self.crcFunc(written)).encode("ascii") + b"\n"
                )
                with self.assertRaises(ResponseTimeoutError):
                    asc.write_register(*args, timeout=0.01)
                fileMock.write.assert_called_once_with(written)
                fileMock.write.reset_mock()
    """
