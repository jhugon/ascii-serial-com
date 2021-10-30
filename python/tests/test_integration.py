import logging
import unittest
from asciiserialcom.host import Host
from asciiserialcom.device import Device
import trio
import trio.testing
from asciiserialcom.utilities import breakStapledIntoWriteRead

logging.basicConfig(
    # filename="test_integration.log",
    # level=logging.INFO,
    # level=logging.DEBUG,
    format="%(asctime)s %(levelname)s L%(lineno)d %(funcName)s: %(message)s"
)


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
            device = Device(
                nursery,
                dev_read_stream,
                dev_write_stream,
                nRegisterBits,
                nRegisters,
                devicePrintRegistersInterval,
            )
            host = Host(nursery, host_read_stream, host_write_stream, nRegisterBits)
            for iReg in range(nRegisters):
                await host.write_register(iReg, 0)
                read_result = await host.read_register(iReg)
                self.assertEqual(read_result, 0)
            for iReg in range(nRegisters):
                await host.write_register(iReg, iReg)
            for iReg in range(nRegisters):
                read_result = await host.read_register(iReg)
                self.assertEqual(read_result, iReg)
            for iReg in range(nRegisters):
                await host.write_register(iReg, 0xFFFFFFFF - iReg)
            for iReg in range(nRegisters):
                read_result = await host.read_register(iReg)
                self.assertEqual(read_result, 0xFFFFFFFF - iReg)
            got_to_cancel = True
            cancel_scope.cancel()
    self.assertTrue(got_to_cancel)


class TestMessageLoopback(unittest.TestCase):
    def test_run(self):
        trio.run(run, self)
