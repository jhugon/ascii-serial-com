import datetime
import os
import os.path
import subprocess
import random
import unittest
import logging
from asciiserialcom.host import Host
from asciiserialcom.errors import *
from asciiserialcom.utilities import (
    MemoryWriteStream,
    MemoryReadStream,
    ChannelWriteStream,
    ChannelReadStream,
    breakStapledIntoWriteRead,
    Tracer,
)
from asciiserialcom.tty_utils import setup_tty
import trio
import serial
import serial.threaded
import queue
import time
import threading
from functools import partial
import termios


class TestRxCounterFromDevice(unittest.TestCase):
    """
    Requires firmware:

        avrdude -p atmega328p -c arduino -P /dev/ttyACM0 -Uflash:w:build/avr5_gcc_debug/arduino_uno_write_pattern_to_serial

    """

    def setUp(self):
        self.baud = 9600
        self.dev_path = "/dev/ttyACM0"
        # self.dev_path = "/dev/ttyACM1"

    def test_just_device(self):
        async def receiver(chan, f):
            while True:
                data = await f.read(1)
                if len(data) == 0:
                    continue
                await chan.send(data)

        async def data_checker(self, chan, data_checker_finished):
            last = None
            logging.debug("Starting data_checker")
            for i in range(400):
                await chan.receive()
            for i in range(2000):
                data = await chan.receive()
                num = int(data)
                logging.debug(f"{i:5} {num:2}")
                if last == 9:
                    self.assertEqual(num, 0)
                elif last:
                    self.assertEqual(num, last + 1)
                last = num
            data_checker_finished.set()

        async def run_test(self):
            logging.info("Starting run_test")
            send_chan, recv_chan = trio.open_memory_channel(100)
            data_checker_finished = trio.Event()
            got_to_cancel = False
            with trio.move_on_after(10):
                logging.info("About to open file...")
                async with await trio.open_file(self.dev_path, "br") as portr:
                    setup_tty(portr.wrapped, self.baud)
                    termios.tcflush(portr.wrapped, termios.TCIFLUSH)
                    logging.debug("flushed input buffer")
                    await portr.read(400)
                    async with trio.open_nursery() as nursery:
                        nursery.start_soon(receiver, send_chan, portr)
                        nursery.start_soon(
                            data_checker, self, recv_chan, data_checker_finished
                        )
                        await data_checker_finished.wait()
                        nursery.cancel_scope.cancel()
            self.assertTrue(data_checker_finished.is_set())

        trio.run(run_test, self)

    def test_just_device_pyserial(self):
        # print("Starting")
        with serial.Serial(self.dev_path, self.baud, 8, "N", 1, timeout=0.1) as ser:
            # print(ser)
            last = None
            ser.reset_input_buffer()
            ser.read(100)
            for i in range(2000):
                data = ser.read(1)
                if len(data) == 0:
                    continue
                num = int(data)
                logging.debug(f"{i:5} {num:2}")
                if last == 9:
                    self.assertEqual(num, 0)
                elif last:
                    self.assertEqual(num, last + 1)
                last = num

    def test_just_device_trio_and_pyserial(self):
        async def receiver(chan, ser):
            f = trio.wrap_file(ser)
            while True:
                # data = await trio.to_thread.run_sync(ser.read,1)
                data = await f.read(1)
                if len(data) == 0:
                    continue
                await chan.send(data)

        async def data_checker(self, chan, chan_all_done):
            last = None
            for i in range(2000):
                if i > 2:
                    data = await chan.receive()
                    num = int(data)
                    # print(f"{i:5} {num:2}")
                    if last == 9:
                        self.assertEqual(num, 0)
                    elif last:
                        self.assertEqual(num, last + 1)
                    last = num
            await chan_all_done.send("All done!")

        async def run_test(self):
            # logging.info("Starting run_test")
            with serial.Serial(self.dev_path, self.baud, 8, "N", 1, timeout=0.1) as ser:
                ser.reset_input_buffer()
                for i in range(5):
                    ser.read(100)
                send_chan, recv_chan = trio.open_memory_channel(100)
                send_chan_all_done, recv_chan_all_done = trio.open_memory_channel(0)
                got_to_cancel = False
                with trio.move_on_after(10) as cancel_scope:
                    async with trio.open_nursery() as nursery:
                        nursery.start_soon(receiver, send_chan, ser)
                        nursery.start_soon(
                            data_checker, self, recv_chan, send_chan_all_done
                        )
                        await recv_chan_all_done.receive()
                        got_to_cancel = True
                        cancel_scope.cancel()
                self.assertTrue(got_to_cancel)

        trio.run(run_test, self)


class TestRxMessageFromDevice(unittest.TestCase):
    """
    Requires firmware:

        avrdude -p atmega328p -c arduino -P /dev/ttyACM0 -Uflash:w:build/avr5_gcc_opt/arduino_uno_write_stream_message_to_serial

    """

    def setUp(self):
        self.baud = 9600
        # self.baud = 19200
        self.dev_path = "/dev/ttyACM0"
        # self.dev_path = "/dev/ttyACM1"

    def test_just_device(self):
        async def receiver(chan, f):
            while True:
                data = await f.read(1)
                if len(data) == 0:
                    continue
                await chan.send(data)

        async def data_checker(self, chan, data_checker_finished):
            last = None
            logging.debug("Starting data_checker",)
            for i in range(400):
                await chan.receive()
            for iMessage in range(10):
                message = b""
                messageStarted = False
                while True:
                    data = await chan.receive()
                    if data == b"\n":
                        if messageStarted:
                            message += data
                            break
                        else:
                            messageStarted = True
                    elif messageStarted:
                        message += data
                logging.debug(f"message received: {message}")
                self.assertEqual(message, b">00s0 0 0 0.DE10\n")
            data_checker_finished.set()

        async def run_test(self):
            logging.info("Starting run_test")
            send_chan, recv_chan = trio.open_memory_channel(100)
            data_checker_finished = trio.Event()
            with trio.move_on_after(10) as cancel_scope:
                logging.info("About to open file...")
                async with await trio.open_file(self.dev_path, "br") as portr:
                    setup_tty(portr.wrapped, self.baud)
                    termios.tcflush(portr.wrapped, termios.TCIFLUSH)
                    logging.debug("flushed input buffer")
                    await portr.read(400)
                    async with trio.open_nursery() as nursery:
                        nursery.start_soon(receiver, send_chan, portr)
                        nursery.start_soon(
                            data_checker, self, recv_chan, data_checker_finished
                        )
                        await data_checker_finished.wait()
                        nursery.cancel_scope.cancel()
            self.assertTrue(data_checker_finished)

        trio.run(run_test, self)


class TestCharLoopback(unittest.TestCase):
    """
        Requires firmware:

        avrdude -p atmega328p -c arduino -P /dev/ttyACM0 -Uflash:w:build/avr5_gcc_debug/arduino_uno_char_loopback

    """

    def setUp(self):
        self.baud = 9600
        self.baud = 19200
        self.dev_path = "/dev/ttyACM0"
        # self.dev_path = "/dev/ttyACM1"

        logging.basicConfig(
            # filename="test_asciiSerialCom.log",
            # level=logging.INFO,
            # level=logging.DEBUG,
            level=logging.WARNING,
            # format="%(asctime)s %(levelname)s L%(lineno)d %(funcName)s: %(message)s"
            format="%(message)s",
        )

    def test_just_device_async(self):
        test_string = b"abcdefghijklmnop987654321"

        async def reader(self, ser, task_status=trio.TASK_STATUS_IGNORED):
            logging.debug(f"reader started!")
            with trio.move_on_after(8) as moveon_scope:
                logging.debug(f"About to wrap file")
                async with trio.wrap_file(ser) as port:
                    logging.debug(f"File wrapped, about to send task started!")
                    task_status.started()
                    try:
                        while True:
                            data = b""
                            while True:
                                data = await port.read(1)
                                if len(data) > 0:
                                    break
                            logging.info(f"read: {data}")
                    except Exception as e:
                        logging.error(e)
            logging.debug("Reader exiting")

        async def run_test(self):
            with trio.move_on_after(10) as moveon_scope:
                with serial.Serial(
                    self.dev_path, self.baud, 8, "N", 1, timeout=0.1
                ) as ser:
                    try:
                        async with trio.wrap_file(ser) as async_ser:
                            async with trio.open_nursery() as nursery:
                                await nursery.start(reader, self, ser)
                                # nursery.start_soon(reader, self, ser)
                                await trio.sleep(0.1)
                                for x in test_string:
                                    await trio.sleep(0)
                                    x = chr(x).encode("ascii")
                                    await trio.sleep(0.1)
                                    await async_ser.write(x)
                                    logging.info(f"Write {x!r}")
                                    await trio.sleep(0)
                                await trio.sleep(2)
                                logging.debug("About to cancel")
                                nursery.cancel_scope.cancel()
                                logging.debug("About to cancel with moveon")
                                moveon_scope.cancel()
                    except Exception as e:
                        logging.error(e)

        trio.run(run_test, self)  # , instruments=[Tracer()])

    def test_just_device_async_no_pyserial(self):
        test_string = b"abcdefghijklmnop987654321"
        test_string_len = len(test_string)

        async def reader(self, reader_finished, task_status=trio.TASK_STATUS_IGNORED):
            logging.debug(f"reader started!")
            with trio.move_on_after(8) as moveon_scope:
                logging.debug(f"About to open file")
                async with await trio.open_file(self.dev_path, "br") as port:
                    logging.debug(f"File open, about to send task started!")
                    task_status.started()
                    try:
                        received_data = b""
                        while True:
                            data = await port.read(1)
                            if len(data) > 0:
                                received_data += data
                                received_data_len = len(received_data)
                                self.assertEqual(
                                    received_data, test_string[:received_data_len]
                                )
                                if received_data_len == test_string_len:
                                    break
                                logging.info(f"read: {data}")
                    except Exception as e:
                        logging.error(e)
            logging.debug("Reader exiting")
            reader_finished.set()

        async def run_test(self):
            reader_finished = trio.Event()
            writer_finished = trio.Event()
            with trio.move_on_after(10) as moveon_scope:
                try:
                    async with await trio.open_file(self.dev_path, "bw") as async_ser:
                        setup_tty(async_ser.wrapped, self.baud)
                        async with trio.open_nursery() as nursery:
                            await nursery.start(reader, self, reader_finished)
                            for x in test_string:
                                await trio.sleep(0)
                                x = chr(x).encode("ascii")
                                await trio.sleep(0)
                                await async_ser.write(x)
                                await async_ser.flush()
                                logging.info(f"Write {x!r}")
                                await trio.sleep(0)
                            writer_finished.set()
                            await reader_finished.wait()
                except Exception as e:
                    logging.error(e)
            self.assertTrue(reader_finished.is_set())
            self.assertTrue(writer_finished.is_set())

        trio.run(run_test, self)  # , instruments=[Tracer()])

    def test_just_device_async_no_pyserial_big_write(self):
        test_string = b"abcdefghijklmnop987654321" * 4 * 100
        test_string_len = len(test_string)
        logging.debug(f"test_string_len: {test_string_len}")

        async def reader(self, reader_finished, task_status=trio.TASK_STATUS_IGNORED):
            logging.debug(f"reader started!")
            with trio.move_on_after(8) as moveon_scope:
                logging.debug(f"About to open file")
                async with await trio.open_file(self.dev_path, "br") as port:
                    logging.debug(f"File open, about to send task started!")
                    task_status.started()
                    try:
                        received_data = b""
                        while True:
                            data = await port.read(1)
                            if len(data) > 0:
                                received_data += data
                                received_data_len = len(received_data)
                                self.assertEqual(
                                    received_data, test_string[:received_data_len]
                                )
                                if received_data_len == test_string_len:
                                    break
                                logging.info(f"read: {data}")
                    except Exception as e:
                        logging.error(e)
            logging.debug("Reader exiting")
            reader_finished.set()

        async def run_test(self):
            reader_finished = trio.Event()
            writer_finished = trio.Event()
            with trio.move_on_after(10) as moveon_scope:
                try:
                    async with await trio.open_file(self.dev_path, "bw") as async_ser:
                        setup_tty(async_ser.wrapped, self.baud)
                        async with trio.open_nursery() as nursery:
                            await nursery.start(reader, self, reader_finished)
                            await async_ser.write(test_string)
                            await async_ser.flush()
                            logging.debug(f"Write {test_string!r}")
                            await trio.sleep(0)
                            writer_finished.set()
                            await reader_finished.wait()
                except Exception as e:
                    logging.error(e)
            self.assertTrue(reader_finished.is_set())
            self.assertTrue(writer_finished.is_set())

        trio.run(run_test, self)  # , instruments=[Tracer()])

    def test_just_device_threads(self):
        class MyProtocol(serial.threaded.Protocol):
            def __init__(self, tester):
                super().__init__()
                self.tester = tester
                self.receive_buffer = queue.Queue()
                self.handle_received_data_thread = threading.Thread(
                    target=self.handle_received_data
                )
                self.handle_received_data_thread.start()

            def data_received(self, data):
                self.receive_buffer.put(data, block=False)

            def handle_received_data(self):
                logging.debug(f"Thread started!")
                last = None
                while True:
                    try:
                        data = self.receive_buffer.get(timeout=0.1)
                    except queue.Empty:
                        pass
                    else:
                        logging.info(f"Received data: {data!r}")
                        num = int(data)
                        if last == 9:
                            self.tester.assertEqual(num, 0)
                        elif last:
                            self.tester.assertEqual(num, last + 1)
                        last = num
                        self.receive_buffer.task_done()

            def join(self):
                self.receive_buffer.join()

        with serial.Serial(
            self.dev_path, self.baud, 8, "N", 1, timeout=0.1, dsrdtr=False
        ) as ser:
            with serial.threaded.ReaderThread(
                ser, partial(MyProtocol, self)
            ) as protocol:
                num = 0
                time.sleep(1)
                # for i in list(range(10)) * 10:
                for i in list(range(10)):
                    data = b"%u" % num
                    ser.write(data)
                    logging.info(f"Write {num}")
                    time.sleep(0.01)
                    if num == 9:
                        num = 0
                    else:
                        num += 1
                time.sleep(1)
                protocol.join()