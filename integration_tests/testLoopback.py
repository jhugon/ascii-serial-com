import os
import os.path
import subprocess
import sys
import pty
import unittest
from asciiserialcom.asciiSerialCom import Ascii_Serial_Com
from asciiserialcom.ascErrors import *


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
        stderrAll = b""
        for intext in [
            b">abc.C103\n",
            b">AFw0123456789.A86F\n",
            b">defxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.350F\n",
        ]:
            with subprocess.Popen(
                [self.exe, "-l"],
                env=self.env,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            ) as proc:
                stdout = b""
                stderr = b""
                try:
                    stdout, stderr = proc.communicate(intext, 0.01)
                except subprocess.TimeoutExpired as e:
                    stdout = e.stdout
                    stderr = e.stderr
                stderrAll += stderr
                self.assertEqual(intext, stdout)
                proc.terminate()


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
            intexts[0] * 20,
            intexts[2] * 2,
            intexts[2] * 5,
        ]
        for intext in intexts:
            with subprocess.Popen(
                [self.exe],
                env=self.env,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            ) as proc:
                stdout = b""
                stderr = b""
                try:
                    stdout, stderr = proc.communicate(intext, 0.1)
                except subprocess.TimeoutExpired as e:
                    stdout = e.stdout
                    stderr = e.stderr
                print("Output:")
                print(stdout)
                print(stderr)
                stderrAll += stderr
                self.assertEqual(intext, stdout)
                proc.terminate()
