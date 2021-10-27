import unittest
from asciiserialcom.asciiSerialCom import Ascii_Serial_Com
from asciiserialcom.device import deviceLoop
import crcmod
import datetime
import trio
import trio.testing
import subprocess
from .testAsciiSerialCom import breakStapledIntoWriteRead


async def run(self):
    nRegisterBits = 32
    nRegisters = 20
    devicePrintRegistersInterval = 10
    host, device = trio.testing.memory_stream_pair()
    host_write_stream, host_read_stream = breakStapledIntoWriteRead(host)
    dev_write_stream, dev_read_stream = breakStapledIntoWriteRead(device)
    got_to_cancel = False
    with trio.move_on_after(1) as cancel_scope:
        async with trio.open_nursery() as nursery:
            nursery.start_soon(
                deviceLoop,
                dev_read_stream,
                dev_write_stream,
                nRegisterBits,
                nRegisters,
                devicePrintRegistersInterval,
            )
            asc = Ascii_Serial_Com(
                nursery, host_read_stream, host_write_stream, nRegisterBits
            )
            for iReg in range(nRegisters):
                await asc.write_register(iReg, 0)
                read_result = await asc.read_register(iReg)
                self.assertEqual(read_result, 0)
            for iReg in range(nRegisters):
                await asc.write_register(iReg, iReg)
            for iReg in range(nRegisters):
                read_result = await asc.read_register(iReg)
                self.assertEqual(read_result, iReg)
            for iReg in range(nRegisters):
                await asc.write_register(iReg, 0xFFFFFFFF - iReg)
            for iReg in range(nRegisters):
                read_result = await asc.read_register(iReg)
                self.assertEqual(read_result, 0xFFFFFFFF - iReg)
            got_to_cancel = True
            cancel_scope.cancel()
    self.assertTrue(got_to_cancel)


class TestMessageLoopback(unittest.TestCase):
    def setUp(self):
        self.crcFunc = crcmod.predefined.mkPredefinedCrcFun("crc-16-dnp")

    def test_run(self):
        trio.run(run, self)
