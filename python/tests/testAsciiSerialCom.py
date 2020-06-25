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
