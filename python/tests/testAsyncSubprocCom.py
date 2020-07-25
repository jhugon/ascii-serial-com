import unittest
import unittest.mock
from unittest.mock import patch
import datetime
import subprocess
import os

from asciiserialcom.asyncSubprocCom import (
    FileReaderThread,
    Async_Subproc_Com,
)


class TestFileReaderThread(unittest.TestCase):
    def test(self):
        with subprocess.Popen(
            ["cat"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            bufsize=0,
            close_fds=True,
        ) as proc:
            frt = FileReaderThread(proc.stdout)
            frt.start()
            for wdata in [b"", b"a", b"abcdefg", b"x" * 50]:
                proc.stdin.write(wdata)
                tstart = datetime.datetime.now()
                data = bytearray()
                while datetime.datetime.now() < tstart + datetime.timedelta(
                    milliseconds=20
                ):
                    data += frt.receive()
                # print("Got data: '{}'".format(data.decode("UTF-8")),flush=True)
                self.assertEqual(wdata, data)
            proc.terminate()


class TestAsync_Subproc_Com(unittest.TestCase):
    def test(self):
        with Async_Subproc_Com(["cat"]) as aspc:
            for wdata in [b"", b"a", b"abcdefg", b"x" * 50]:
                aspc.send(wdata)
                tstart = datetime.datetime.now()
                data = bytearray()
                while datetime.datetime.now() < tstart + datetime.timedelta(
                    milliseconds=20
                ):
                    data += aspc.receive()
                # print("Got data: '{}'".format(data.decode("UTF-8")), flush=True)
                self.assertEqual(wdata, data)
