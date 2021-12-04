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


class TestRxCounterFromDevice(unittest.TestCase):
    """
    Requires firmware:

        openocd -f /usr/share/openocd/scripts/board/st_nucleo_f0.cfg -c "program build/cortex-m0_gcc_debug/stm32f091nucleo64_write_pattern_to_serial.elf verify reset exit"

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
        setup_tty(self.dev_path, self.baud)

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


class TestRxASCCounterFromDevice(unittest.TestCase):
    """
    Requires firmware:

        openocd -f /usr/share/openocd/scripts/board/st_nucleo_f0.cfg -c "program build/cortex-m0_gcc_debug/stm32f091nucleo64_adc_streaming.elf verify reset exit"

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
        setup_tty(self.dev_path, self.baud)
        self.max32bits = 2 ** 32 - 1

    def test_host_device(self):
        nRegisterBits = 32

        async def data_checker(self, chan, data_checker_finished):
            logging.debug("Starting data_checker",)
            last = None
            starttime = time.time()
            nMessages = 200
            messageLen = 0
            for iMessage in range(nMessages):
                nMissed, data = await chan.receive()
                logging.info(f"message received: {nMissed} {data}")
                num = int(data.decode()[0], 16)
                if last == self.max32bits:
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
                        logging.debug("files opened")
                        async with trio.open_nursery() as nursery:
                            logging.debug("About to startup host")
                            host = Host(nursery, portr, portw, nRegisterBits)
                            logging.debug("Host started")
                            host.forward_received_s_messages_to(send_chan)
                            logging.debug("messages now forwarded to send_chan")
                            nursery.start_soon(
                                data_checker, self, recv_chan, data_checker_finished
                            )
                            await host.write_register(2, 1)  # set to counter mode
                            await host.start_streaming()
                            logging.debug(
                                "data_checker starting soon, waiting for it to finish"
                            )
                            await data_checker_finished.wait()
                            logging.debug("data_checker_finished")
                            await host.stop_streaming()
                            await host.write_register(
                                2, 0
                            )  # set back to default ADC mode
                            nursery.cancel_scope.cancel()
            self.assertTrue(data_checker_finished.is_set())

        trio.run(run_test, self)
