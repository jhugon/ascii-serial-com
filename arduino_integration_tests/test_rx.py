import datetime
import unittest
import logging
from asciiserialcom.message import ASC_Message
from asciiserialcom.host import Host
from asciiserialcom.errors import *
from asciiserialcom.utilities import Tracer
from asciiserialcom.tty_utils import setup_tty
import trio
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

        logging.basicConfig(
            # filename="test_asciiSerialCom.log",
            # level=logging.INFO,
            # level=logging.DEBUG,
            level=logging.WARNING,
            # format="%(asctime)s %(levelname)s L%(lineno)d %(funcName)s: %(message)s"
            format="%(message)s",
        )

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

        logging.basicConfig(
            # filename="test_asciiSerialCom.log",
            # level=logging.INFO,
            # level=logging.DEBUG,
            level=logging.WARNING,
            # format="%(asctime)s %(levelname)s L%(lineno)d %(funcName)s: %(message)s"
            format="%(message)s",
        )

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

    def test_host_device(self):
        nRegisterBits = 32

        async def data_checker(self, chan, data_checker_finished):
            logging.debug("Starting data_checker",)
            for iMessage in range(10):
                messageStarted = False
                msg = await chan.receive()
                logging.debug(f"message received: {msg}")
                data = msg.data
                self.assertEqual(data, b"0 0 0 0")
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
                    logging.debug("TTY setup and flushed")
                    await portr.read(400)
                    logging.debug("Read dummy data from input")
                    async with trio.open_nursery() as nursery:
                        logging.debug("About to startup host")
                        host = Host(nursery, portr, None, nRegisterBits)
                        logging.debug("Host started")
                        host.forward_received_s_messages_to(send_chan)
                        logging.debug("messages now forwarded to send_chan")
                        nursery.start_soon(
                            data_checker, self, recv_chan, data_checker_finished
                        )
                        logging.debug(
                            "data_checker starting soon, waiting for it to finish"
                        )
                        await data_checker_finished.wait()
                        logging.debug("data_checker_finished")
                        nursery.cancel_scope.cancel()
            self.assertTrue(data_checker_finished.is_set())

        trio.run(run_test, self)
