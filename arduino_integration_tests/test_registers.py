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
import random


class TestDummyRegisterBlock(unittest.TestCase):
    """
    Requires firmware:

        avrdude -p atmega328p -c arduino -P /dev/ttyACM0 -Uflash:w:build/avr5_gcc_debug/arduino_uno_dummy_register_block

    """

    def setUp(self):
        self.baud = 9600
        self.dev_path = "/dev/ttyACM0"
        # self.dev_path = "/dev/ttyACM1"

        logging.basicConfig(
            # filename="test_asciiSerialCom.log",
            # level=logging.INFO,
            # level=logging.DEBUG,
            # level=logging.WARNING,
            format="%(asctime)s %(levelname)s L%(lineno)d %(funcName)s: %(message)s"
        )

    def test_write_read(self):
        nRegisterBits = 8
        nRegisters = 10

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
                            for iReg in range(nRegisters):
                                await host.write_register(iReg, iReg)
                                content = await host.read_register(iReg)
                                logging.debug(f"Register {iReg} read: {content}")
                                self.assertEqual(content, iReg)
                            for iReg in range(nRegisters):
                                content = await host.read_register(iReg)
                                logging.debug(f"Register {iReg} read: {content}")
                                self.assertEqual(content, iReg)
                            for iReg in range(nRegisters):
                                await host.write_register(iReg, nRegisters - iReg)
                                content = await host.read_register(iReg)
                                logging.debug(f"Register {iReg} read: {content}")
                                self.assertEqual(content, nRegisters - iReg)
                            for iTry in range(2):
                                vals = [
                                    random.randint(0, 255) for i in range(nRegisters)
                                ]
                                for iReg in range(nRegisters):
                                    await host.write_register(iReg, vals[iReg])
                                for iReg in range(nRegisters):
                                    val = await host.read_register(iReg)
                                    self.assertEqual(val, vals[iReg])
                            all_finished.set()
                            nursery.cancel_scope.cancel()
            self.assertTrue(all_finished.is_set())

        trio.run(run_test, self)
