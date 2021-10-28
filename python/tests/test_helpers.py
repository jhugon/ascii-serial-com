import unittest
import unittest.mock
from unittest.mock import patch
from asciiserialcom.helpers import *
from asciiserialcom.message import ASC_Message
from asciiserialcom.errors import *
import crcmod
import datetime


class TestChecks(unittest.TestCase):
    def test_check_register_content(self):

        for nBits in [8, 16, 32, 64]:
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
                    self.assertEqual(check_register_content(arg, nBits), comp)

            for arg in [-3, 2.4, b"0" * (nBits // 4 + 1), b"0" * 200]:
                with self.subTest(i="nBits={}, arg={}".format(nBits, arg)):
                    with self.assertRaises(BadRegisterContentError):
                        check_register_content(arg, nBits)

    def test_check_register_number(self):

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
                self.assertEqual(check_register_number(arg), comp)

        for arg in [-3, 2.4, 0x1FFFF, b"0" * 5, b"0" * 200]:
            with self.subTest(i="arg={}".format(arg)):
                with self.assertRaises(BadRegisterNumberError):
                    check_register_number(arg)


class TestConvert(unittest.TestCase):
    def test_to_hex(self):
        self.assertEqual(convert_to_hex(b"e"), b"0E")
        self.assertEqual(convert_to_hex(b"e", 5), b"0000E")
        self.assertEqual(convert_to_hex(b"e", 0), b"E")
        self.assertEqual(convert_to_hex(b"e", 1), b"E")

        self.assertEqual(convert_to_hex("e"), b"0E")
        self.assertEqual(convert_to_hex("e", 5), b"0000E")
        self.assertEqual(convert_to_hex("e", 0), b"E")
        self.assertEqual(convert_to_hex("e", 1), b"E")

        self.assertEqual(convert_to_hex(bytearray(b"e")), b"0E")
        self.assertEqual(convert_to_hex(bytearray(b"e"), 5), b"0000E")
        self.assertEqual(convert_to_hex(bytearray(b"e"), 0), b"E")
        self.assertEqual(convert_to_hex(bytearray(b"e"), 1), b"E")

        self.assertEqual(convert_to_hex(255), b"FF")
        self.assertEqual(convert_to_hex(255, 5), b"000FF")
        self.assertEqual(convert_to_hex(255, 0), b"FF")
        self.assertEqual(convert_to_hex(255, 1), b"FF")

        self.assertEqual(convert_to_hex(0), b"00")
        self.assertEqual(convert_to_hex(0, 5), b"00000")
        self.assertEqual(convert_to_hex(0, 0), b"0")
        self.assertEqual(convert_to_hex(0, 1), b"0")

        for x in (b"", "", bytearray(b"")):
            with self.assertRaises(ValueError):
                convert_to_hex(x)

    def test_from_hex(self):
        self.assertEqual(convert_from_hex(b"e"), 14)
        self.assertEqual(convert_from_hex(bytearray(b"e")), 14)
        self.assertEqual(convert_from_hex("e"), 14)

        self.assertEqual(convert_from_hex(b"123"), 291)
        self.assertEqual(convert_from_hex(bytearray(b"123")), 291)
        self.assertEqual(convert_from_hex("123"), 291)

        self.assertEqual(convert_from_hex(b"0" * 20), 0)

        for x in (b"", "", bytearray(b"")):
            with self.assertRaises(ValueError):
                convert_from_hex(x)

        for x in (b"g", b"135x235", "x125"):
            with self.assertRaises(ValueError):
                convert_from_hex(x)
