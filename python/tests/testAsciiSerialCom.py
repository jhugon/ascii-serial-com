import unittest
import unittest.mock
from asciiserialcom.asciiSerialCom import Ascii_Serial_Com
import crcmod


class TestReadReg(unittest.TestCase):
    def setUp(self):
        self.fileMock = unittest.mock.MagicMock()
        self.asc = Ascii_Serial_Com(self.fileMock, 32)
        self.crcFunc = crcmod.predefined.mkPredefinedCrcFun("crc-16-dnp")

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
