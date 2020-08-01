import crcmod
import datetime
import os
import os.path
import subprocess
import random
import unittest
from asciiserialcom.asciiSerialCom import Ascii_Serial_Com
from asciiserialcom.ascErrors import *
from asciiserialcom.comSubproc import Com_Subproc


class TestTrivialLoopback(unittest.TestCase):
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
        with Com_Subproc([self.exe], env=self.env, hideStderr=True) as comSubproc:
            for i in range(10):
                regNum = random.randrange(10)
                regVal = random.randrange(256)
                wmessage = ">00w{:04X},{:02X}.".format(regNum, regVal).encode("ascii")
                wmessage += (
                    "{:04X}".format(self.crcFunc(wmessage)).encode("ascii") + b"\n"
                )
                rmessage = ">00r{:04X}.".format(regNum).encode("ascii")
                rmessage += (
                    "{:04X}".format(self.crcFunc(rmessage)).encode("ascii") + b"\n"
                )
                wresponse_check = ">00w{:04X}.".format(regNum).encode("ascii")
                wresponse_check += (
                    "{:04X}".format(self.crcFunc(wresponse_check)).encode("ascii")
                    + b"\n"
                )
                rresponse_check = ">00r{:04X},{:02X}.".format(regNum, regVal).encode(
                    "ascii"
                )
                rresponse_check += (
                    "{:04X}".format(self.crcFunc(rresponse_check)).encode("ascii")
                    + b"\n"
                )

                ### Test Write
                comSubproc.send(wmessage)
                tstart = datetime.datetime.now()
                wresponse = bytearray()
                while datetime.datetime.now() < tstart + datetime.timedelta(
                    milliseconds=20
                ):
                    wresponse += comSubproc.receive()
                # print("Got w response: '{}'".format(wresponse),flush=True)
                self.assertEqual(wresponse_check, wresponse)
                ### Test Readback
                comSubproc.send(rmessage)
                tstart = datetime.datetime.now()
                rresponse = bytearray()
                while datetime.datetime.now() < tstart + datetime.timedelta(
                    milliseconds=20
                ):
                    rresponse += comSubproc.receive()
                # print("Got r response: '{}'".format(rresponse),flush=True)
                self.assertEqual(rresponse_check, rresponse)

    def test_host_device(self):
        with subprocess.Popen(
            [self.exe],
            env=self.env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            bufsize=0,
            close_fds=True,
        ) as proc:
            asc = Ascii_Serial_Com(proc.stdout, proc.stdin, 8)
            for i in range(250):
                regNum = random.randrange(10)
                regVal = random.randrange(256)
                asc.write_register(regNum, regVal)
                regVal_read_bytes = asc.read_register(regNum)
                regVal_read = int(regVal_read_bytes, 16)
                self.assertEqual(regVal, regVal_read)

            for i in range(3):
                vals = random.choices(range(256), k=10)
                for regNum, regVal in enumerate(vals):
                    asc.write_register(regNum, regVal)
                for regNum, regVal in enumerate(vals):
                    regVal_read_bytes = asc.read_register(regNum)
                    regVal_read = int(regVal_read_bytes, 16)
                    self.assertEqual(regVal, regVal_read)
