import datetime
import os
import os.path
import subprocess
import random
import unittest
from asciiserialcom.asciiSerialCom import Ascii_Serial_Com
from asciiserialcom.ascErrors import *
from asciiserialcom.utilities import (
    MemoryWriteStream,
    MemoryReadStream,
    ChannelWriteStream,
    ChannelReadStream,
    breakStapledIntoWriteRead,
)
import trio

alphabytes = b"abcdefghijklmnopqrstuvwxyz"
alphanumeric = alphabytes + bytes(alphabytes).upper() + b"0123456789"


class TestTrivialLoopback(unittest.TestCase):
    def setUp(self):
        self.env = os.environ.copy()
        platform = "native"
        CC = "gcc"
        build_type = "debug"
        self.env.update({"platform": platform, "CC": CC, "build_type": build_type})
        self.exedir = "build/{}_{}_{}".format(platform, CC, build_type)
        self.exe = os.path.join(self.exedir, "ascii_serial_com_dummy_loopback_device")
        random.seed(123456789)

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

        async def run_test(self, intexts):
            got_to_cancel = False
            with trio.move_on_after(1) as cancel_scope:
                async with await trio.open_process(
                    [self.exe, "-l"],
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.DEVNULL,
                ) as device:
                    for intext in intexts:
                        await device.stdio.send_all(intext)
                        data = await device.stdio.receive_some()
                        self.assertEqual(data, intext)
                    got_to_cancel = True
                    cancel_scope.cancel()
            self.assertTrue(got_to_cancel)

        trio.run(run_test, self, intexts)

    def test_host_device(self):
        async def run_test(self):
            nRegisterBits = 32
            got_to_cancel = False
            with trio.move_on_after(1) as cancel_scope:
                async with await trio.open_process(
                    [self.exe, "-l"],
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.DEVNULL,
                ) as device:
                    host_w, host_r = breakStapledIntoWriteRead(device.stdio)
                    send_chan, recv_chan = trio.open_memory_channel(0)
                    async with trio.open_nursery() as nursery:
                        asc = Ascii_Serial_Com(nursery, host_r, host_w, nRegisterBits)
                        asc.forward_all_received_messages_to(send_chan)
                        for testCommand in [b"a", b"b", b"c"]:
                            for testData in [b"", b"abcdefg", b"x" * 54]:
                                await asc.send_message(testCommand, testData)
                                msg = await recv_chan.receive()
                                self.assertEqual(msg.command, testCommand)
                                self.assertEqual(msg.data, testData)
                        got_to_cancel = True
                        cancel_scope.cancel()
            self.assertTrue(got_to_cancel)

        trio.run(run_test, self)

    def test_host_device_random(self):
        async def run_test(self):
            nRegisterBits = 32
            got_to_cancel = False
            with trio.move_on_after(1) as cancel_scope:
                async with await trio.open_process(
                    [self.exe, "-l"],
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.DEVNULL,
                ) as device:
                    host_w, host_r = breakStapledIntoWriteRead(device.stdio)
                    send_chan, recv_chan = trio.open_memory_channel(0)
                    async with trio.open_nursery() as nursery:
                        asc = Ascii_Serial_Com(nursery, host_r, host_w, nRegisterBits)
                        asc.forward_all_received_messages_to(send_chan)
                        for i in range(1000):
                            testCommand = bytes([random.choice(alphabytes)])
                            nData = random.randrange(55)
                            testData = bytes(random.choices(alphanumeric, k=nData))
                            await asc.send_message(testCommand, testData)
                            msg = await recv_chan.receive()
                            self.assertEqual(msg.command, testCommand)
                            self.assertEqual(msg.data, testData)
                        got_to_cancel = True
                        cancel_scope.cancel()
            self.assertTrue(got_to_cancel)

        trio.run(run_test, self)


class TestASCLoopback(unittest.TestCase):
    def setUp(self):
        self.env = os.environ.copy()
        platform = "native"
        CC = "gcc"
        build_type = "debug"
        self.env.update({"platform": platform, "CC": CC, "build_type": build_type})
        self.exedir = "build/{}_{}_{}".format(platform, CC, build_type)
        self.exe = os.path.join(self.exedir, "ascii_serial_com_dummy_loopback_device")
        random.seed(123456789)

    def test_just_host(self):
        async def run_test(self):
            nRegisterBits = 32
            got_to_cancel = False
            with trio.move_on_after(1) as cancel_scope:
                dev_send_chan, dev_recv_chan = trio.open_memory_channel(0)
                receiver_send_chan, receiver_recv_chan = trio.open_memory_channel(0)
                async with trio.open_nursery() as nursery:
                    asc = Ascii_Serial_Com(
                        nursery,
                        ChannelReadStream(dev_recv_chan),
                        ChannelWriteStream(dev_send_chan),
                        nRegisterBits,
                    )
                    asc.forward_all_received_messages_to(receiver_send_chan)
                    for testCommand in [b"a", b"b", b"c"]:
                        for testData in [b"", b"abcdefg", b"x" * 54]:
                            await asc.send_message(testCommand, testData)
                            msg = await receiver_recv_chan.receive()
                            self.assertEqual(msg.command, testCommand)
                            self.assertEqual(msg.data, testData)
                    got_to_cancel = True
                    cancel_scope.cancel()
            self.assertTrue(got_to_cancel)

        trio.run(run_test, self)

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

        async def run_test(self, intexts):
            got_to_cancel = False
            with trio.move_on_after(10) as cancel_scope:
                async with await trio.open_process(
                    [self.exe],
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    # stderr=subprocess.DEVNULL,
                ) as device:
                    for intext in intexts:
                        await device.stdio.send_all(intext)
                        response = b""
                        with trio.move_on_after(0.1) as cancel_scope:
                            while True:
                                response += await device.stdio.receive_some()
                        self.assertEqual(response, intext)
                    got_to_cancel = True
                    cancel_scope.cancel()
            self.assertTrue(got_to_cancel)

        trio.run(run_test, self, intexts)

    def test_just_device_badframes(self):
        intexts = [
            b">abc.C103",
            b">abcC103\n",
            b">AFw0123456789.086F\n",
        ]

        with Com_Subproc([self.exe], env=self.env, hideStderr=True) as comSubproc:
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
            for testCommand in [b"a", b"b", b"c"]:
                for testData in [b"", b"abcdefg", b"x" * 54]:
                    asc.send_message(testCommand, testData)
                    msg = asc.receive_message()
                    self.assertEqual(testCommand, msg.command)
                    self.assertEqual(testData, msg.data)

    def test_host_device_random(self):
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
            for i in range(10000):
                testCommand = bytes([random.choice(alphabytes)])
                nData = random.randrange(55)
                testData = bytes(random.choices(alphanumeric, k=nData))
                # print(testCommand,testData)
                asc.send_message(testCommand, testData)
                msg = asc.receive_message()
                self.assertEqual(testCommand, msg.command)
                self.assertEqual(testData, msg.data)
