import unittest
import unittest.mock
from unittest.mock import patch
from asciiserialcom.asciiSerialCom import send_message, Ascii_Serial_Com
from asciiserialcom.ascMessage import ASC_Message
from asciiserialcom.circularBuffer import Circular_Buffer_Bytes
from asciiserialcom.ascErrors import *
from asciiserialcom.utilities import breakStapledIntoWriteRead, MemoryWriteStream
import crcmod
import datetime
import trio
import trio.testing


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

    def test_read_reg(self):
        async def run_func(send_chan, func, *args):
            async with send_chan:
                result = await func(*args)
                await send_chan.send(result)

        async def run_test(self, reg_num, reg_val):
            dev_reply_message = b">00r%04X,%08X." % (reg_num, reg_val)
            dev_reply_message += (
                "{:04X}".format(self.crcFunc(dev_reply_message)).encode("ascii") + b"\n"
            )
            dev_expect_message = b">00r%04X." % (reg_num)
            dev_expect_message += (
                "{:04X}".format(self.crcFunc(dev_expect_message)).encode("ascii")
                + b"\n"
            )

            host, device = trio.testing.memory_stream_pair()
            host_write_stream, host_read_stream = breakStapledIntoWriteRead(host)
            got_to_cancel = False
            with trio.move_on_after(0.5) as cancel_scope:
                result_send_chan, result_recv_chan, = trio.open_memory_channel(0)
                async with result_recv_chan:
                    async with trio.open_nursery() as nursery:
                        asc = Ascii_Serial_Com(
                            nursery, host_read_stream, host_write_stream, 32
                        )
                        nursery.start_soon(
                            run_func, result_send_chan, asc.read_register, reg_num
                        )
                        dev_receive_message = await device.receive_some()
                        self.assertEqual(dev_receive_message, dev_expect_message)
                        await device.send_all(dev_reply_message)
                        result = await result_recv_chan.receive()
                        self.assertEqual(result, reg_val)
                        got_to_cancel = True
                        cancel_scope.cancel()
            self.assertTrue(got_to_cancel)

        for reg_num, reg_val in [(2, 0x1234567A), (0xFF, 0)]:
            with self.subTest(i="reg_num={}, reg_val={}".format(reg_num, reg_val)):
                trio.run(run_test, self, reg_num, reg_val)

    def test_write_reg(self):
        async def run_func(send_chan, func, *args):
            async with send_chan:
                result = await func(*args)
                await send_chan.send(result)

        async def run_test(self, args, written, read, nRegBits):
            written += "{:04X}".format(self.crcFunc(written)).encode("ascii") + b"\n"
            read += "{:04X}".format(self.crcFunc(read)).encode("ascii") + b"\n"
            host, device = trio.testing.memory_stream_pair()
            host_write_stream, host_read_stream = breakStapledIntoWriteRead(host)
            got_to_cancel = False
            with trio.move_on_after(0.5) as cancel_scope:
                result_send_chan, result_recv_chan, = trio.open_memory_channel(0)
                async with result_recv_chan:
                    async with trio.open_nursery() as nursery:
                        asc = Ascii_Serial_Com(
                            nursery, host_read_stream, host_write_stream, nRegBits
                        )
                        nursery.start_soon(
                            run_func, result_send_chan, asc.write_register, *args
                        )
                        dev_receive_message = await device.receive_some()
                        self.assertEqual(dev_receive_message, written)
                        await device.send_all(read)
                        result = await result_recv_chan.receive()
                        self.assertEqual(
                            result, None
                        )  # b/c write returns non on success but want to check it does
                        got_to_cancel = True
                        cancel_scope.cancel()
            self.assertTrue(got_to_cancel)

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
                trio.run(run_test, self, args, written, read, 8)

        for args, written in [
            ((b"0", b"0000"), b">00w0000,00000000."),
            ((b"FF", b"E3E3"), b">00w00FF,0000E3E3."),
            ((b"FFFF", b"1F1F1F1F"), b">00wFFFF,1F1F1F1F."),
            ((0, 0), b">00w0000,00000000."),
            ((0xFF, 0xE3), b">00w00FF,000000E3."),
            ((0xFFFF, 0x1F1F1F1F), b">00wFFFF,1F1F1F1F."),
        ]:
            read = written[:-10] + b"."
            with self.subTest(
                i="args={}, written={} read={}".format(args, written, read)
            ):
                trio.run(run_test, self, args, written, read, 32)
