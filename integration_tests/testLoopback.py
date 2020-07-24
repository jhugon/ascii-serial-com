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
from asciiserialcom.asyncSubprocCom import (
    Async_Subproc_Com,
    FileReaderThread,
    FileWriterThread,
)


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
        #        stderrAll = b""
        #        for intext in [
        #            b">abc.C103\n",
        #            b">AFw0123456789.A86F\n",
        #            b">defxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.350F\n",
        #        ]:
        #            with subprocess.Popen(
        #                [self.exe, "-l"],
        #                env=self.env,
        #                stdin=subprocess.PIPE,
        #                stdout=subprocess.PIPE,
        #                stderr=subprocess.PIPE,
        #            ) as proc:
        #                stdout = b""
        #                stderr = b""
        #                try:
        #                    stdout, stderr = proc.communicate(intext, 0.01)
        #                except subprocess.TimeoutExpired as e:
        #                    stdout = e.stdout
        #                    stderr = e.stderr
        #                stderrAll += stderr
        #                self.assertEqual(intext, stdout)
        #                proc.terminate()

        return
        with subprocess.Popen(
            [self.exe, "-l"],
            env=self.env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            bufsize=0,
            close_fds=True,
        ) as proc:
            frt = FileReaderThread(proc.stdout)
            frt.start()
            for intext in [
                b">abc.C103\n",
                b">AFw0123456789.A86F\n",
                b">defxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.350F\n",
            ]:
                proc.stdin.write(intext)
                tstart = datetime.datetime.now()
                data = bytearray()
                while datetime.datetime.now() < tstart + datetime.timedelta(
                    milliseconds=20
                ):
                    data += frt.get_data()
                # print("Got data: '{}'".format(data.decode("UTF-8")),flush=True)
                self.assertEqual(intext, data)
            proc.terminate()


#    def test_python_and_device(self):
#        devr, hostw = os.pipe()
#        hostr, devw = os.pipe()
#        with os.fdopen(hostw,"a") as hostwFile:
#            with subprocess.Popen(
#                [self.exe, "-l"],
#                env=self.env,
#                stdin=devr,
#                #stdout=devw,
#            ) as proc:
#                hostw.write(b"abcdefg\n")
#                print(devr,hostw,hostr,devw)
#                print(type(devr))
#                print(hostwFile)
#                proc.terminate()
#
#
#
#        #asc = Ascii_Serial_Com(hostr,hostw,32)
#        #with subprocess.Popen(
#        #    [self.exe, "-l"],
#        #    env=self.env,
#        #    stdin=devr,
#        #    stdout=devw,
#        #) as proc:
#        #    #asc.send_message(b"w",b"01010101")
#        #    proc.terminate()


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
        ]
        intexts = [intexts[4]]
        for intext in intexts:
            # print("Input:")
            # print(intext.decode("ASCII"))
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
                    # print("TimeoutExpired!")
                    stdout = e.stdout
                    stderr = e.stderr
                # print("Output:")
                # print(stdout.decode("ASCII"))
                print("Stderr:")
                print(stderr.decode("ASCII"), flush=True)
                stderrAll += stderr
                self.assertEqual(intext, stdout)
                proc.terminate()

        for intext in intexts:
            with subprocess.Popen(
                [self.exe],
                env=self.env,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                bufsize=0,
                # close_fds=True,
            ) as proc:
                frt = FileReaderThread(proc.stdout)
                fwt = FileWriterThread(proc.stdin)
                frt.start()
                fwt.start()
                # proc.stdin.write(intext)
                fwt.push_data(intext)
                fwt.push_data(b"")
                tstart = datetime.datetime.now()
                data = bytearray()
                while datetime.datetime.now() < tstart + datetime.timedelta(
                    milliseconds=1000
                ):
                    data += frt.get_data()
                    time.sleep(0.1)
                print("Got data: '{}'".format(data), flush=True)
                self.assertEqual(intext, data)
                # proc.terminate()

        # with subprocess.Popen(
        #    [self.exe],
        #    env=self.env,
        #    stdin=subprocess.PIPE,
        #    stdout=subprocess.PIPE,
        #    bufsize=0,
        #    close_fds=True,
        # ) as proc:
        #    frt = FileReaderThread(proc.stdout)
        #    frt.start()
        #    for intext in intexts:
        #        proc.stdin.write(intext)
        #        tstart = datetime.datetime.now()
        #        data = bytearray()
        #        while datetime.datetime.now() < tstart + datetime.timedelta(
        #            milliseconds=200
        #        ):
        #            data += frt.get_data()
        #        # print("Got data: '{}'".format(data.decode("UTF-8")),flush=True)
        #        self.assertEqual(intext, data)
        #    proc.terminate()
