import unittest
import unittest.mock
from unittest.mock import patch
from asciiserialcom.asciiSerialCom import Ascii_Serial_Com
from asciiserialcom.ascErrors import *
import crcmod
import datetime


class TestConvert(unittest.TestCase):
    def setUp(self):
        self.fileMock = unittest.mock.MagicMock()
        self.asc = Ascii_Serial_Com(self.fileMock, self.fileMock, 32)

    def test_to_hex(self):
        asc = self.asc
        self.assertEqual(asc._convert_to_hex(b"e"), b"0E")
        self.assertEqual(asc._convert_to_hex(b"e", 5), b"0000E")
        self.assertEqual(asc._convert_to_hex(b"e", 0), b"E")
        self.assertEqual(asc._convert_to_hex(b"e", 1), b"E")

        self.assertEqual(asc._convert_to_hex("e"), b"0E")
        self.assertEqual(asc._convert_to_hex("e", 5), b"0000E")
        self.assertEqual(asc._convert_to_hex("e", 0), b"E")
        self.assertEqual(asc._convert_to_hex("e", 1), b"E")

        self.assertEqual(asc._convert_to_hex(bytearray(b"e")), b"0E")
        self.assertEqual(asc._convert_to_hex(bytearray(b"e"), 5), b"0000E")
        self.assertEqual(asc._convert_to_hex(bytearray(b"e"), 0), b"E")
        self.assertEqual(asc._convert_to_hex(bytearray(b"e"), 1), b"E")

        self.assertEqual(asc._convert_to_hex(255), b"FF")
        self.assertEqual(asc._convert_to_hex(255, 5), b"000FF")
        self.assertEqual(asc._convert_to_hex(255, 0), b"FF")
        self.assertEqual(asc._convert_to_hex(255, 1), b"FF")

        self.assertEqual(asc._convert_to_hex(0), b"00")
        self.assertEqual(asc._convert_to_hex(0, 5), b"00000")
        self.assertEqual(asc._convert_to_hex(0, 0), b"0")
        self.assertEqual(asc._convert_to_hex(0, 1), b"0")

        for x in (b"", "", bytearray(b"")):
            with self.assertRaises(ValueError):
                asc._convert_to_hex(x)

    def test_from_hex(self):
        asc = self.asc

        self.assertEqual(asc._convert_from_hex(b"e"), 14)
        self.assertEqual(asc._convert_from_hex(bytearray(b"e")), 14)
        self.assertEqual(asc._convert_from_hex("e"), 14)

        self.assertEqual(asc._convert_from_hex(b"123"), 291)
        self.assertEqual(asc._convert_from_hex(bytearray(b"123")), 291)
        self.assertEqual(asc._convert_from_hex("123"), 291)

        self.assertEqual(asc._convert_from_hex(b"0" * 20), 0)

        for x in (b"", "", bytearray(b"")):
            with self.assertRaises(ValueError):
                asc._convert_from_hex(x)

        for x in (b"g", b"135x235", "x125"):
            with self.assertRaises(ValueError):
                asc._convert_from_hex(x)


class TestCRC(unittest.TestCase):
    def setUp(self):
        self.fileMock = unittest.mock.MagicMock()
        self.asc = Ascii_Serial_Com(self.fileMock, self.fileMock, 32)
        self.crcFunc = crcmod.predefined.mkPredefinedCrcFun("crc-16-dnp")

    def test_crc(self):
        reg_num = 2
        reg_val = 0x1234567A
        reply_message = b">00r%02X,%08X." % (reg_num, reg_val)
        good_crc = "{:02X}".format(self.crcFunc(reply_message)).encode("ascii")
        self.assertEqual(self.asc._compute_checksum(reply_message), good_crc)
        self.assertEqual(
            self.asc._compute_checksum(reply_message + b"aslkdgasbv\n"), good_crc
        )

    def test_crc_raises(self):
        asc = self.asc
        for x in (b"", b"1251235", b">235235", b".", b"235.", b"22235\n", b"\n", b">"):
            with self.assertRaises(MalformedFrameError):
                asc._compute_checksum(x)


class TestChecks(unittest.TestCase):
    def setUp(self):
        self.fileMock = unittest.mock.MagicMock()

    def test_check_command(self):
        asc = Ascii_Serial_Com(self.fileMock, self.fileMock, 32)

        for i in [b"w", b"W", bytearray(b"W"), "W"]:
            with self.subTest(i=i):
                self.assertEqual(asc._check_command(i), b"w")

        for i in [b"www", b"", 3, b"2"]:
            with self.subTest(i=i):
                with self.assertRaises(BadCommandError):
                    asc._check_command(i)

    def test_check_data(self):
        asc = Ascii_Serial_Com(self.fileMock, self.fileMock, 32)

        for i in [
            (b"12345", b"12345"),
            ("12345", b"12345"),
            (bytearray(b"12345"), b"12345"),
            (b"", b""),
            (b"0" * 56, b"0" * 56),
        ]:
            with self.subTest(i=i):
                self.assertEqual(asc._check_data(b"w", i[0]), i[1])

        for i in [3, 2.4, b"0" * 57, b"0" * 200]:
            with self.subTest(i=i):
                with self.assertRaises(BadDataError):
                    asc._check_data(b"w", i)

    def test_check_register_content(self):

        for nBits in [8, 16, 32, 64]:
            asc = Ascii_Serial_Com(self.fileMock, self.fileMock, nBits)
            for arg, comp in [
                (b"0", b"0"),
                (b"FF", b"FF"),
                (bytearray(b"ff"), b"FF"),
                ("ff", b"FF"),
                (0xFF, b"FF"),
                (0x3A, b"3A"),
                (3, b"3"),
            ]:
                lencomp = nBits // 4 - len(comp)
                if lencomp > 0:
                    comp = b"0" * lencomp + comp
                with self.subTest(i="nBits={}, arg={}".format(nBits, arg)):
                    self.assertEqual(asc._check_register_content(arg), comp)

            for arg in [-3, 2.4, b"0" * (nBits // 4 + 1), b"0" * 200]:
                with self.subTest(i="nBits={}, arg={}".format(nBits, arg)):
                    with self.assertRaises(BadRegisterContentError):
                        asc._check_register_content(arg)

    def test_check_register_number(self):

        asc = Ascii_Serial_Com(self.fileMock, self.fileMock, 32)
        for arg, comp in [
            (b"0", b"0000"),
            (b"FF", b"00FF"),
            (bytearray(b"ff"), b"00FF"),
            ("ff", b"00FF"),
            (0xFF, b"00FF"),
            (0x3A, b"003A"),
            (3, b"0003"),
        ]:
            with self.subTest(i="arg={}".format(arg)):
                self.assertEqual(asc._check_register_number(arg), comp)

        for arg in [-3, 2.4, 0x1FFFF, b"0" * 5, b"0" * 200]:
            with self.subTest(i="arg={}".format(arg)):
                with self.assertRaises(BadRegisterNumberError):
                    asc._check_register_number(arg)


class TestFramingAndStreaming(unittest.TestCase):
    def setUp(self):
        self.fileMock = unittest.mock.MagicMock()
        self.crcFunc = crcmod.predefined.mkPredefinedCrcFun("crc-16-dnp")
        now = datetime.datetime.now()
        self.times10ms = [
            now + datetime.timedelta(seconds=i / 100.0) for i in range(100)
        ]
        self.times100ms = [
            now + datetime.timedelta(seconds=i / 10.0) for i in range(100)
        ]
        self.times1s = [now + datetime.timedelta(seconds=i) for i in range(100)]

    def test_pack_message(self):
        asc = Ascii_Serial_Com(self.fileMock, self.fileMock, 32)

        for args, returnval in [
            ((b"w", b""), b">00w."),
            ((b"w", b"0"), b">00w0."),
            ((b"w", b"00000000"), b">00w00000000."),
        ]:
            with self.subTest(i="args={}, returnval={}".format(args, returnval)):
                returnval += (
                    "{:02X}".format(self.crcFunc(returnval)).encode("ascii") + b"\n"
                )
                self.assertEqual(asc._pack_message(*args), returnval)

    def test_unpack_message(self):
        asc = Ascii_Serial_Com(self.fileMock, self.fileMock, 32)

        for frame, returnval in [
            (b">00w.", (b"0", b"0", b"w", b"")),
            (b">09x.", (b"0", b"9", b"x", b"")),
            (b">00w0123456789ABCDEF.", (b"0", b"0", b"w", b"0123456789ABCDEF")),
            (b">0Fw" + b"A" * 56 + b".", (b"0", b"F", b"w", b"A" * 56)),
        ]:
            with self.subTest(i="frame={}, returnval={}".format(frame, returnval)):
                frame += "{:02X}".format(self.crcFunc(frame)).encode("ascii") + b"\n"
                self.assertEqual(asc._unpack_message(frame), returnval)

        frame = b">\n"
        with self.assertRaises(MalformedFrameError):
            asc._unpack_message(frame)
        frame = b">.\n"
        with self.assertRaises(MessageIntegrityError):
            asc._unpack_message(frame)
        frame = b">."
        frame += "{:02X}".format(self.crcFunc(frame) + 1).encode("ascii") + b"\n"
        with self.assertRaises(MessageIntegrityError):
            asc._unpack_message(frame)

        for frame in [b">.", b">00.", b">0w.", b">w.", b""]:
            with self.subTest(i="frame: {}".format(frame)):
                frame += "{:02X}".format(self.crcFunc(frame)).encode("ascii") + b"\n"
                with self.assertRaises(MalformedFrameError):
                    asc._unpack_message(frame)

    def test_frame_from_stream(self):
        fileMock = self.fileMock

        with unittest.mock.patch("datetime.datetime") as datetimeMock:
            datetimeMock.now.side_effect = self.times10ms

            for filedata, returnval in [
                (b">\n", b">\n"),
                (b">" + b"0" * 10 + b"\n", b">" + b"0" * 10 + b"\n"),
                (b">" + b"0" * 62 + b"\n", b">" + b"0" * 62 + b"\n"),
                (b"0" * 20 + b">" + b"0" * 20 + b"\n", b">" + b"0" * 20 + b"\n"),
                (b">" + b"0" * 20 + b"\n" + b"0" * 20, b">" + b"0" * 20 + b"\n"),
                (
                    b"0" * 20 + b">" + b"0" * 20 + b"\n" + b"0" * 10,
                    b">" + b"0" * 20 + b"\n",
                ),
            ]:
                with self.subTest(
                    i="filedata={}, returnval={}".format(filedata, returnval)
                ):
                    fileMock.read.return_value = filedata
                    asc = Ascii_Serial_Com(fileMock, fileMock, 32)
                    asc.selectorIn = unittest.mock.MagicMock()
                    asc.selectorIn.select.return_value = [0]
                    self.assertEqual(asc._frame_from_stream(0.1), returnval)
                    fileMock.reset_mock()
                    datetimeMock.now.reset_mock()

            for filedata in [
                b"0" * 10,
                b">" + b"0" * 10,
                b"0" * 10 + b"\n",
                b">" * 10,
                b"\n" * 10,
                # b">" + b"0" * 63 + b"\n",
            ]:
                with self.subTest(i="filedata={}".format(filedata)):
                    fileMock.read.return_value = filedata
                    asc = Ascii_Serial_Com(fileMock, fileMock, 32)
                    asc.selectorIn = unittest.mock.MagicMock()
                    asc.selectorIn.select.return_value = [0]
                    self.assertIsNone(asc._frame_from_stream(0.1))
                    fileMock.reset_mock()
                    datetimeMock.now.reset_mock()


class TestMessaging(unittest.TestCase):
    def setUp(self):
        self.crcFunc = crcmod.predefined.mkPredefinedCrcFun("crc-16-dnp")

    def test_receive_message(self):
        fileMock = unittest.mock.MagicMock()
        asc = Ascii_Serial_Com(fileMock, fileMock, 32)
        asc.selectorIn = unittest.mock.MagicMock()
        asc.selectorIn.select.return_value = [0]

        for frame, returnval in [
            (b">00w.", (b"0", b"0", b"w", b"")),
            (b">09x.", (b"0", b"9", b"x", b"")),
            (b">00w0123456789ABCDEF.", (b"0", b"0", b"w", b"0123456789ABCDEF")),
            (b">0Fw" + b"A" * 56 + b".", (b"0", b"F", b"w", b"A" * 56)),
        ]:
            with self.subTest(i="frame={}, returnval={}".format(frame, returnval)):
                frame += "{:02X}".format(self.crcFunc(frame)).encode("ascii") + b"\n"
                fileMock.read.return_value = frame
                self.assertEqual(asc.receive_message(), returnval)

        for frame in [b">10w.", b">F0w."]:
            with self.subTest(i="frame: {}".format(frame)):
                frame += "{:02X}".format(self.crcFunc(frame)).encode("ascii") + b"\n"
                fileMock.read.return_value = frame
                with self.assertRaises(AsciiSerialComVersionMismatchError):
                    asc.receive_message()

    def test_send_message(self):
        fileMock = unittest.mock.MagicMock()
        asc = Ascii_Serial_Com(fileMock, fileMock, 32)

        for frame, args in [
            (b">00w.", (b"w", b"")),
            (b">00w0123456789ABCDEF.", (b"w", b"0123456789ABCDEF")),
            (b">00w" + b"A" * 56 + b".", (b"w", b"A" * 56)),
        ]:
            with self.subTest(i="frame={}, args={}".format(frame, args)):
                frame += "{:02X}".format(self.crcFunc(frame)).encode("ascii") + b"\n"
                asc.send_message(*args)
                fileMock.write.assert_called_once_with(frame)
                fileMock.write.reset_mock()

    def test_read_reg(self):
        fileMock = unittest.mock.MagicMock()
        asc = Ascii_Serial_Com(fileMock, fileMock, 32)
        asc.selectorIn = unittest.mock.MagicMock()
        asc.selectorIn.select.return_value = [0]
        for reg_num, reg_val in [(2, 0x1234567A), (0xFF, 0)]:
            with self.subTest(i="reg_num={}, reg_val={}".format(reg_num, reg_val)):
                reply_message = b">00r%04X,%08X." % (reg_num, reg_val)
                reply_message += (
                    "{:02X}".format(self.crcFunc(reply_message)).encode("ascii") + b"\n"
                )
                fileMock.read.return_value = reply_message
                result = asc.read_register(reg_num)
                self.assertEqual(result, b"%08X" % reg_val)
                write_message = b">00r%04X." % (reg_num)
                write_message += (
                    "{:02X}".format(self.crcFunc(write_message)).encode("ascii") + b"\n"
                )
                fileMock.write.assert_called_once_with(write_message)
                fileMock.write.reset_mock()

                # reply with wrong reg number
                reply_message = b">00r%04X,%08X." % (reg_num + 1, reg_val)
                reply_message += (
                    "{:02X}".format(self.crcFunc(reply_message)).encode("ascii") + b"\n"
                )
                fileMock.read.return_value = reply_message
                with self.assertRaises(ResponseTimeoutError):
                    asc.read_register(reg_num, timeout=0.01)
                fileMock.write.assert_called_once_with(write_message)
                fileMock.write.reset_mock()

        fileMock.read.return_value = b""
        with self.assertRaises(ResponseTimeoutError):
            asc.read_register(2, timeout=0.01)

    def test_write_reg(self):
        fileMock = unittest.mock.MagicMock()
        asc = Ascii_Serial_Com(fileMock, fileMock, 8)
        asc.selectorIn = unittest.mock.MagicMock()
        asc.selectorIn.select.return_value = [0]

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
                    "{:02X}".format(self.crcFunc(written)).encode("ascii") + b"\n"
                )
                read += "{:02X}".format(self.crcFunc(read)).encode("ascii") + b"\n"
                fileMock.read.return_value = read
                asc.write_register(*args)
                fileMock.write.assert_called_once_with(written)
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
                    "{:02X}".format(self.crcFunc(written)).encode("ascii") + b"\n"
                )
                fileMock.read.return_value = b""
                with self.assertRaises(ResponseTimeoutError):
                    asc.write_register(*args, timeout=0.01)
                fileMock.write.assert_called_once_with(written)
                fileMock.write.reset_mock()
