import crcmod
import datetime
import os
import os.path
import subprocess
import random
import unittest
from asciiserialcom.host import Host
from asciiserialcom.errors import *
from asciiserialcom.utilities import (
    MemoryWriteStream,
    MemoryReadStream,
    ChannelWriteStream,
    ChannelReadStream,
    breakStapledIntoWriteRead,
)
import trio


class TestRegisterReadback(unittest.TestCase):
    def setUp(self):
        self.env = os.environ.copy()
        platform = "native"
        CC = "gcc"
        build_type = "debug"
        self.env.update({"platform": platform, "CC": CC, "build_type": build_type})
        self.exedir = "build/{}_{}_{}".format(platform, CC, build_type)
        self.exe = os.path.join(self.exedir, "ascii_serial_com_dummy_register_device")
        random.seed(123456789)
        self.crcFunc = crcmod.predefined.mkPredefinedCrcFun("crc-16-dnp")

    def test_just_device(self):
        async def run_test(self):
            got_to_cancel = False
            with trio.move_on_after(5) as cancel_scope:
                async with await trio.open_process(
                    [self.exe],
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.DEVNULL,
                ) as device:
                    for i in range(10):
                        regNum = random.randrange(10)
                        regVal = random.randrange(256)
                        wmessage = ">00w{:04X},{:02X}.".format(regNum, regVal).encode(
                            "ascii"
                        )
                        wmessage += (
                            "{:04X}".format(self.crcFunc(wmessage)).encode("ascii")
                            + b"\n"
                        )
                        rmessage = ">00r{:04X}.".format(regNum).encode("ascii")
                        rmessage += (
                            "{:04X}".format(self.crcFunc(rmessage)).encode("ascii")
                            + b"\n"
                        )
                        wresponse_check = ">00w{:04X}.".format(regNum).encode("ascii")
                        wresponse_check += (
                            "{:04X}".format(self.crcFunc(wresponse_check)).encode(
                                "ascii"
                            )
                            + b"\n"
                        )
                        rresponse_check = ">00r{:04X},{:02X}.".format(
                            regNum, regVal
                        ).encode("ascii")
                        rresponse_check += (
                            "{:04X}".format(self.crcFunc(rresponse_check)).encode(
                                "ascii"
                            )
                            + b"\n"
                        )

                        """
                        await device.stdio.send_all(intext)
                        data = await device.stdio.receive_some()
                        self.assertEqual(data, intext)
                        """

                        ### Test Write
                        await device.stdio.send_all(wmessage)
                        wresponse = await device.stdio.receive_some()
                        # print("Got w response: '{}'".format(wresponse),flush=True)
                        self.assertEqual(wresponse_check, wresponse)
                        ### Test Readback
                        await device.stdio.send_all(rmessage)
                        rresponse = await device.stdio.receive_some()
                        # print("Got r response: '{}'".format(rresponse),flush=True)
                        self.assertEqual(rresponse_check, rresponse)
                    got_to_cancel = True
                    cancel_scope.cancel()
            self.assertTrue(got_to_cancel)

        trio.run(run_test, self)

    def test_host_device_8bit(self):
        async def run_test(self):
            nRegisterBits = 8
            testDataMax = 2 ** nRegisterBits
            got_to_cancel = False
            with trio.move_on_after(5) as cancel_scope:
                async with await trio.open_process(
                    [self.exe],
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.DEVNULL,
                ) as device:
                    host_w, host_r = breakStapledIntoWriteRead(device.stdio)
                    async with trio.open_nursery() as nursery:
                        host = Host(nursery, host_r, host_w, nRegisterBits)
                        for testRegNum in range(10):
                            for testData in range(testDataMax):
                                await host.write_register(testRegNum, testData)
                                result = await host.read_register(testRegNum)
                                self.assertEqual(result, testData)
                        got_to_cancel = True
                        cancel_scope.cancel()
            self.assertTrue(got_to_cancel)

        trio.run(run_test, self)
