import unittest
import unittest.mock
from asciiserialcom.asciiSerialCom import Ascii_Serial_Com
from asciiserialcom.ascErrors import *
import crcmod


class TestConvert(unittest.TestCase):
    def setUp(self):
        self.fileMock = unittest.mock.MagicMock()
        self.asc = Ascii_Serial_Com(self.fileMock, 32)

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
        self.asc = Ascii_Serial_Com(self.fileMock, 32)
        self.crcFunc = crcmod.predefined.mkPredefinedCrcFun("crc-16-dnp")

    def test_crc(self):
        reg_num = 2
        reg_val = 0x1234567A
        reply_message = b">00r%02X,%08X." % (reg_num, reg_val)
        good_crc = hex(self.crcFunc(reply_message)).upper().encode("ascii")
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
        asc = Ascii_Serial_Com(self.fileMock, 32)

        for i in [b"w", b"W", bytearray(b"W"), "W"]:
            with self.subTest(i=i):
                self.assertEqual(asc._check_command(i), b"w")

        for i in [b"www", b"", 3, b"2"]:
            with self.subTest(i=i):
                with self.assertRaises(BadCommandError):
                    asc._check_command(i)

    def test_check_data(self):
        asc = Ascii_Serial_Com(self.fileMock, 32)

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
            asc = Ascii_Serial_Com(self.fileMock, nBits)
            for arg, comp in [
                (b"0", b"0"),
                (b"FF", b"FF"),
                (bytearray(b"ff"), b"FF"),
                ("ff", b"FF"),
                (0xFF, b"FF"),
                (0x3A, b"3A"),
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


class TestFramingAndStreaming(unittest.TestCase):
    def setUp(self):
        self.fileMock = unittest.mock.MagicMock()
        self.asc = Ascii_Serial_Com(self.fileMock, 32)

    def test_pack_message(self):
        pass

    def test_unpack_message(self):
        pass

    def test_frame_from_message(self):
        pass


class TestMessaging(unittest.TestCase):
    def setUp(self):
        self.fileMock = unittest.mock.MagicMock()
        self.asc = Ascii_Serial_Com(self.fileMock, 32)
        self.crcFunc = crcmod.predefined.mkPredefinedCrcFun("crc-16-dnp")

    def test_receive_message(self):
        pass

    def test_send_message(self):
        pass

    def test_read_reg(self):
        reg_num = 2
        reg_val = 0x1234567A
        reply_message = b">00r%02X,%08X." % (reg_num, reg_val)
        reply_message += (
            hex(self.crcFunc(reply_message)).upper().encode("ascii") + b"\n"
        )
        fileMock = self.fileMock
        fileMock.read.return_value = reply_message
        asc = self.asc
        result = asc.read_register(2)
        self.assertEqual(result, b"%08X" % reg_val)

    def test_write_reg(self):
        pass
