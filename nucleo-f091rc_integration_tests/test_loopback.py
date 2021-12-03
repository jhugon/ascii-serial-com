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


class TestCharLoopback(unittest.TestCase):
    """
        Requires firmware:

        openocd -f /usr/share/openocd/scripts/board/st_nucleo_f0.cfg -c "program build/cortex-m0_gcc_debug/stm32f091nucleo64_char_loopback.elf verify reset

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
            format="%(levelname)s L%(lineno)d %(funcName)s: %(message)s",
        )
        setup_tty(self.dev_path, self.baud)

    def test_just_device(self):
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
                                logging.debug(f"read: {data}")
                                received_data += data
                                received_data_len = len(received_data)
                                self.assertEqual(
                                    received_data, test_string[:received_data_len]
                                )
                                if received_data_len == test_string_len:
                                    break
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
                        async with trio.open_nursery() as nursery:
                            await nursery.start(reader, self, reader_finished)
                            for x in test_string:
                                await trio.sleep(0)
                                x = chr(x).encode("ascii")
                                await trio.sleep(0)
                                await async_ser.write(x)
                                await async_ser.flush()
                                logging.debug(f"Write {x!r}")
                                await trio.sleep(0)
                            writer_finished.set()
                            await reader_finished.wait()
                except Exception as e:
                    logging.error(e)
            self.assertTrue(reader_finished.is_set())
            self.assertTrue(writer_finished.is_set())

        trio.run(run_test, self)  # , instruments=[Tracer()])

    def test_just_device_big_write(self):
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
                                logging.debug(f"read: {data}")
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
                    raise e
            self.assertTrue(reader_finished.is_set())
            self.assertTrue(writer_finished.is_set())

        trio.run(run_test, self)  # , instruments=[Tracer()])

    def test_host_device(self):
        """
        Send ASC messages to device and read them back
        """
        nRegisterBits = 32
        messages = [
            ASC_Message(b"0", b"0", b"r", b"00"),
            ASC_Message(b"0", b"0", b"r", b"04FF"),
            ASC_Message(b"0", b"0", b"w", b"04FF,00000000"),
            ASC_Message(b"0", b"0", b"w", b"04FF,FFFFFFFF"),
            ASC_Message(b"0", b"0", b"s", b"04FF FFFFFFFF"),
        ] * 10

        async def data_checker(
            self, chan, data_checker_finished, task_status=trio.TASK_STATUS_IGNORED
        ):
            logging.debug("Starting data_checker")
            task_status.started()
            for iMessage in range(len(messages)):
                msg = await chan.receive()
                logging.debug(f"message received: {msg}")
                self.assertEqual(msg, messages[iMessage])
            data_checker_finished.set()

        async def run_test(self):
            logging.info("Starting run_test")
            send_chan, recv_chan = trio.open_memory_channel(100)
            data_checker_finished = trio.Event()
            with trio.move_on_after(10) as cancel_scope:
                logging.info("About to open file...")
                async with await trio.open_file(self.dev_path, "br") as portr:
                    async with await trio.open_file(self.dev_path, "bw") as portw:
                        termios.tcflush(portr.wrapped, termios.TCIFLUSH)
                        logging.debug("TTY setup and flushed")
                        async with trio.open_nursery() as nursery:
                            logging.debug("About to startup host")
                            host = Host(nursery, portr, portw, nRegisterBits)
                            logging.debug("Host started")
                            host.forward_all_received_messages_to(send_chan)
                            logging.debug("messages now forwarded to send_chan")
                            await nursery.start(
                                data_checker, self, recv_chan, data_checker_finished
                            )
                            logging.debug("data_checker started")
                            for msg in messages:
                                await host.send_message(msg.command, msg.data)
                            await data_checker_finished.wait()
                            logging.debug("data_checker_finished")
                            nursery.cancel_scope.cancel()
            self.assertTrue(data_checker_finished.is_set())

        trio.run(run_test, self)


class TestASCLoopback(unittest.TestCase):
    """
        Requires firmware:

        openocd -f /usr/share/openocd/scripts/board/st_nucleo_f0.cfg -c "program build/cortex-m0_gcc_debug/stm32f091nucleo64_asc_loopback.elf verify reset

    """

    def setUp(self):
        self.baud = 9600
        self.dev_path = "/dev/ttyACM0"
        # self.dev_path = "/dev/ttyACM1"

        logging.basicConfig(
            # filename="test_asciiSerialCom.log",
            level=logging.INFO,
            # level=logging.DEBUG,
            # level=logging.WARNING,
            format="%(levelname)s L%(lineno)d %(funcName)s: %(message)s",
        )
        setup_tty(self.dev_path, self.baud)

    def test_host_device(self):
        """
        Send ASC messages to device and read them back
        """
        nRegisterBits = 32
        messages = [
            ASC_Message(b"0", b"0", b"r", b"00"),
            ASC_Message(b"0", b"0", b"r", b"04FF"),
            ASC_Message(b"0", b"0", b"w", b"04FF,00000000"),
            ASC_Message(b"0", b"0", b"w", b"04FF,FFFFFFFF"),
            ASC_Message(b"0", b"0", b"s", b"04FF FFFFFFFF"),
        ] * 10

        async def data_checker(
            self, chan, data_checker_finished, task_status=trio.TASK_STATUS_IGNORED
        ):
            logging.debug("Starting data_checker")
            task_status.started()
            try:
                for iMessage in range(len(messages)):
                    logging.debug(f"Trying to receive message {iMessage}")
                    msg = await chan.receive()
                    logging.debug(f"message {iMessage} received: {msg}")
                    self.assertEqual(msg, messages[iMessage])
                data_checker_finished.set()
            except trio.EndOfChannel as e:
                logging.error("Receive channel closed unexpectedly")

        async def run_test(self):
            logging.info("Starting run_test")
            send_chan, recv_chan = trio.open_memory_channel(100)
            data_checker_finished = trio.Event()
            with trio.move_on_after(30) as cancel_scope:
                logging.info("About to open file...")
                async with await trio.open_file(self.dev_path, "br") as portr:
                    async with await trio.open_file(self.dev_path, "bw") as portw:
                        termios.tcflush(portr.wrapped, termios.TCIFLUSH)
                        logging.debug("TTY setup and flushed")
                        async with trio.open_nursery() as nursery:
                            logging.debug("About to startup host")
                            host = Host(nursery, portr, portw, nRegisterBits)
                            logging.debug("Host started")
                            host.forward_all_received_messages_to(send_chan)
                            logging.debug("messages now forwarded to send_chan")
                            await nursery.start(
                                data_checker, self, recv_chan, data_checker_finished
                            )
                            logging.debug("data_checker started")
                            for msg in messages:
                                await host.send_message(msg.command, msg.data)
                                await trio.sleep(0.05)
                            await data_checker_finished.wait()
                            logging.debug("data_checker_finished")
                            nursery.cancel_scope.cancel()
            self.assertTrue(data_checker_finished.is_set())

        trio.run(run_test, self)
