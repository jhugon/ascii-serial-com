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
import time


class TestMulti(unittest.TestCase):
    """
    Requires firmware:

        avrdude -p atmega328p -c arduino -P /dev/ttyACM0 -Uflash:w:build/avr5_gcc_opt/arduino_uno_test_device

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
            format="%(levelname)s L%(lineno)d %(funcName)s: %(message)s",
        )

    def test_rx_stream_counter(self):
        nRegisterBits = 32

        async def data_checker(self, chan, data_checker_finished):
            logging.debug("Starting data_checker",)
            last = None
            starttime = time.time()
            nMessages = 500
            for iMessage in range(nMessages):
                messageStarted = False
                nMissed, data = await chan.receive()
                logging.info(f"message received: {nMissed} {data}")
                self.assertEqual(nMissed, 0)
                num = int(data.decode(), 16)
                if last == 255:
                    self.assertEqual(num, 0)
                elif last:
                    self.assertEqual(num, last + 1)
                last = num
                if iMessage == 0:
                    messageLen = len(data) + 10
            endtime = time.time()
            deltat = endtime - starttime
            tpermessage = deltat / nMessages
            tperbyte = tpermessage / messageLen
            bitpers = 1 / tperbyte * 8
            logging.warning(
                f"Time spent per message: {tpermessage} s, per byte: {tperbyte}, per bit: {1./bitpers}"
            )
            logging.warning(
                f"Message per second {1./tpermessage}, byte per second: {1./tperbyte}, bit per second: {bitpers}"
            )
            data_checker_finished.set()

        async def run_test(self):
            logging.info("Starting run_test")
            send_chan, recv_chan = trio.open_memory_channel(100)
            data_checker_finished = trio.Event()
            with trio.move_on_after(10) as cancel_scope:
                logging.info("About to open file...")
                async with await trio.open_file(self.dev_path, "br") as portr:
                    async with await trio.open_file(self.dev_path, "bw") as portw:
                        setup_tty(portr.wrapped, self.baud)
                        termios.tcflush(portr.wrapped, termios.TCIFLUSH)
                        logging.debug("TTY setup and flushed")
                        async with trio.open_nursery() as nursery:
                            logging.debug("About to startup host")
                            host = Host(nursery, portr, portw, nRegisterBits)
                            logging.debug("Host started")
                            await host.stop_streaming()
                            host.forward_received_s_messages_to(send_chan)
                            logging.debug("messages now forwarded to send_chan")
                            await host.start_streaming()
                            nursery.start_soon(
                                data_checker, self, recv_chan, data_checker_finished
                            )
                            logging.debug(
                                "data_checker starting soon, waiting for it to finish"
                            )
                            await data_checker_finished.wait()
                            await host.stop_streaming()
                            logging.debug("data_checker_finished")
                            nursery.cancel_scope.cancel()
            self.assertTrue(data_checker_finished.is_set())

        trio.run(run_test, self)

    def test_register_write_read(self):
        nRegisterBits = 8
        nRegisters = 3
        masks = [1 << 5, 0, 0]

        async def run_test(self):
            logging.info("Starting run_test")
            all_finished = trio.Event()
            with trio.move_on_after(10) as cancel_scope:
                logging.info("About to open file...")
                async with await trio.open_file(self.dev_path, "br") as portr:
                    async with await trio.open_file(self.dev_path, "bw") as portw:
                        setup_tty(portr.wrapped, self.baud)
                        termios.tcflush(portr.wrapped, termios.TCIFLUSH)
                        logging.debug("TTY setup and flushed")
                        async with trio.open_nursery() as nursery:
                            logging.debug("About to startup host")
                            host = Host(nursery, portr, portw, nRegisterBits)
                            logging.debug("Host started")
                            await host.stop_streaming()
                            for iReg in range(nRegisters):
                                await host.write_register(iReg, 0)
                                content = await host.read_register(iReg)
                                self.assertEqual(content, 0)
                            for iReg in range(nRegisters):
                                await host.write_register(iReg, 0xFF)
                                content = await host.read_register(iReg)
                                self.assertEqual(content, masks[iReg])
                            all_finished.set()
                            nursery.cancel_scope.cancel()
            self.assertTrue(all_finished.is_set())

        trio.run(run_test, self)
