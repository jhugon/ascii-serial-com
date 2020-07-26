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

    def test_just_device_communicate(self):

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
        for intext in intexts:
            with subprocess.Popen(
                [self.exe, "-l"],
                # ["cat"],
                env=self.env,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            ) as proc:
                stdout = b""
                stderr = b""
                try:
                    stdout, stderr = proc.communicate(intext, 0.02)
                except subprocess.TimeoutExpired as e:
                    print("TimeoutExpired!")
                    stdout = e.stdout
                    stderr = e.stderr
                print("Output:")
                print(stdout.decode("ASCII"))
                print("Stderr:")
                print(stderr.decode("ASCII"), flush=True)
                self.assertEqual(intext, stdout)
                proc.terminate()

    def test_just_device_com_subproc(self):

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

    def test_just_device_communicate(self):
        intexts = [
            # b">abc.C103",
            b">abc.C103\n",
            b">AFw0123456789.A86F\n",
            b">defxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.350F\n",
        ]
        intexts += [
            intexts[0] * 2,
            intexts[0] * 5,
            (intexts[0] * 20)[:60],
        ]
        # intexts = [intexts[4]]

        for intext in intexts:
            print("Input:", intext)
            with subprocess.Popen(
                [self.exe],
                env=self.env,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                # stderr=subprocess.PIPE,
            ) as proc:
                stdout = b""
                stderr = b""
                try:
                    stdout, stderr = proc.communicate(intext, 0.1)
                except subprocess.TimeoutExpired as e:
                    # print("TimeoutExpired!")
                    stdout = e.stdout
                    stderr = e.stderr
                # print("Output:")
                # print(stdout.decode("ASCII"))
                self.assertEqual(intext, stdout)
                proc.terminate()

    def test_just_device_com_subproc(self):
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
                print("For intext: ", intext)
                comSubproc.send(intext)
                tstart = datetime.datetime.now()
                data = bytearray()
                while datetime.datetime.now() < tstart + datetime.timedelta(
                    milliseconds=20
                ):
                    data += comSubproc.receive()
                print("Got data: '{}'".format(data.decode("UTF-8")), flush=True)
                self.assertEqual(intext, data)
            time.sleep(0.5)
            comSubproc.terminate()  # explicitly terminate since exe doesn't exit on file closes
            time.sleep(0.5)

    def test_just_device_badframes_communicate(self):
        intexts = [
            b">abc.C103",
            b">abcC103\n",
            b">AFw0123456789.086F\n",
        ]

        for intext in intexts:
            # print("Input:",intext)
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
                    stdout, stderr = proc.communicate(intext, 0.2)
                except subprocess.TimeoutExpired as e:
                    # print("TimeoutExpired!")
                    stdout = e.stdout
                    stderr = e.stderr
                # print("Output:")
                # print(stdout.decode("ASCII"))
                # self.assertEqual(b"", stdout)
                self.assertEqual(None, stdout)
                proc.terminate()
