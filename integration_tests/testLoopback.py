import datetime
import os
import os.path
import subprocess
import sys
import time
import pty
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
        self.exe = os.path.join(self.exedir, "ascii_serial_com_dummy_loopback_device")

    def test_just_device(self):

        intexts = [
            b">abc.C103\n",
            b">AFw0123456789.A86F\n",
            b">defxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.350F\n",
        ]
        intexts += [
            intexts[0] * 2,
            intexts[0] * 5,
            (intexts[0] * 20)[:64],
        ]
        with Com_Subproc([self.exe, "-l"], env=self.env) as comSubproc:
            for intext in intexts:
                comSubproc.send(intext)
                tstart = datetime.datetime.now()
                data = bytearray()
                while datetime.datetime.now() < tstart + datetime.timedelta(
                    milliseconds=20
                ):
                    data += comSubproc.receive()
                # print("Got data: '{}'".format(data.decode("UTF-8")),flush=True)
                self.assertEqual(intext, data)
            comSubproc.terminate()  # explicitly terminate since exe doesn't exit on file closes


class TestASCLoopback(unittest.TestCase):
    def setUp(self):
        self.env = os.environ.copy()
        platform = "native"
        CC = "gcc"
        build_type = "debug"
        self.env.update({"platform": platform, "CC": CC, "build_type": build_type})
        self.exedir = "build/{}_{}_{}".format(platform, CC, build_type)
        self.exe = os.path.join(self.exedir, "ascii_serial_com_dummy_loopback_device")

    def test_just_device(self):
        stderrAll = b""
        intexts = [
            b">abc.C103\n",
            b">AFw0123456789.A86F\n",
            b">defxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.350F\n",
        ]
        intexts += [
            intexts[0] * 2,
            intexts[0] * 5,
            (intexts[0] * 20)[:60],
        ]

        with Com_Subproc([self.exe], env=self.env) as comSubproc:
            for intext in intexts:
                # print("For intext: ", intext)
                comSubproc.send(intext)
                tstart = datetime.datetime.now()
                data = bytearray()
                while datetime.datetime.now() < tstart + datetime.timedelta(
                    milliseconds=20
                ):
                    data += comSubproc.receive()
                # print("Got data: '{}'".format(data.decode("UTF-8")), flush=True)
                self.assertEqual(intext, data)

    def test_just_device_badframes(self):
        intexts = [
            b">abc.C103",
            b">abcC103\n",
            b">AFw0123456789.086F\n",
        ]

        with Com_Subproc([self.exe], env=self.env) as comSubproc:
            for intext in intexts:
                # print("For intext: ", intext)
                comSubproc.send(intext)
                tstart = datetime.datetime.now()
                data = bytearray()
                while datetime.datetime.now() < tstart + datetime.timedelta(
                    milliseconds=20
                ):
                    data += comSubproc.receive()
                # print("Got data: '{}'".format(data.decode("UTF-8")), flush=True)
                self.assertEqual(b"", data)
