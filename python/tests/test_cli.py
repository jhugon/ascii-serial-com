import logging
import unittest
import trio
import trio.testing
import os
from pathlib import Path


class TestCLI(unittest.TestCase):
    def setUp(self):
        logging.basicConfig(
            # filename="test_integration.log",
            # level=logging.INFO,
            level=logging.DEBUG,
            format="%(asctime)s %(levelname)s L%(lineno)d %(funcName)s: %(message)s",
        )
        host_to_device_fifo = Path("host_to_device.fifo")
        device_to_host_fifo = Path("device_to_host.fifo")
        if not host_to_device_fifo.exists():
            os.mkfifo(host_to_device_fifo)
        if not device_to_host_fifo.exists():
            os.mkfifo(device_to_host_fifo)
        self.host_to_device_fifo = host_to_device_fifo
        self.device_to_host_fifo = device_to_host_fifo

    def tearDown(self):
        self.host_to_device_fifo.unlink()
        self.device_to_host_fifo.unlink()
        logging.basicConfig(
            # filename="test_integration.log",
            # level=logging.INFO,
            # level=logging.DEBUG,
            format="%(asctime)s %(levelname)s L%(lineno)d %(funcName)s: %(message)s"
        )

    def test_read(self):
        async def sender(self):
            logging.debug(f"sender outfile: {self.device_to_host_fifo}")
            async with await trio.open_file(self.device_to_host_fifo, "wb") as f:
                await trio.sleep(0.5)
                await f.write(b">00r0000,0F.9A58\n")

        async def receiver(self):
            logging.debug(f"receiver infile: {self.host_to_device_fifo}")
            data = b""
            async with await trio.open_file(self.host_to_device_fifo, "rb") as f:
                while True:
                    data += await f.read()
                    await trio.sleep(0.05)
            logging.debug(f"{data!r:4}")
            self.assertEqual(data, b">00r0000.DDA7\n")

        async def run_test(self):
            test_timeout = 5  # seconds
            host_timeout = 2  # seconds
            with trio.move_on_after(test_timeout) as move_on_scope:
                async with trio.open_nursery() as nursery:
                    nursery.start_soon(receiver, self)
                    nursery.start_soon(
                        trio.run_process,
                        [
                            "asciiSerialComCli",
                            "read",
                            str(self.device_to_host_fifo),
                            "0",
                            "--timeout",
                            str(host_timeout),
                            "--serial-send",
                            str(self.host_to_device_fifo),
                        ],
                    )
                    nursery.start_soon(sender, self)
                    for i in range(10):
                        s = ""
                        for t in nursery.child_tasks:
                            s += str(t)
                        logging.debug(f"Tasks still running: {s}")
                        await trio.sleep(1)

        trio.run(run_test, self)

    def test_write(self):
        async def sender(self):
            async with await trio.open_file(self.device_to_host_fifo, "wb") as f:
                await trio.sleep(0.5)
                await f.write(b">00w0000.252E\n")

        async def receiver(self):
            async with await trio.open_file(self.host_to_device_fifo, "rb") as f:
                for i in range(1000):
                    data = await f.read()
                    if len(data) > 0:
                        logging.debug(f"{i:4} {data!r:4}")

        async def run_test(self):
            test_timeout = 10  # seconds
            host_timeout = 5  # seconds
            with trio.move_on_after(test_timeout) as move_on_scope:
                async with trio.open_nursery() as nursery:
                    nursery.start_soon(receiver, self)
                    nursery.start_soon(
                        trio.run_process,
                        [
                            "asciiSerialComCli",
                            "write",
                            str(self.device_to_host_fifo),
                            "0",
                            "255",
                            "--timeout",
                            str(host_timeout),
                            "--serial-send",
                            str(self.host_to_device_fifo),
                        ],
                    )
                    nursery.start_soon(sender, self)

        trio.run(run_test, self)
