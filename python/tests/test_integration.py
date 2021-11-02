import logging
import unittest
from asciiserialcom.host import Host
from asciiserialcom.device import Device
import trio
import trio.testing
import os
from pathlib import Path
from asciiserialcom.utilities import breakStapledIntoWriteRead

logging.basicConfig(
    # filename="test_integration.log",
    # level=logging.INFO,
    # level=logging.DEBUG,
    format="%(asctime)s %(levelname)s L%(lineno)d %(funcName)s: %(message)s"
)


class TestMessageLoopback(unittest.TestCase):
    def test_run(self):
        async def run_test(self):
            nRegisterBits = 32
            nRegisters = 20
            devicePrintRegistersInterval = 10
            host, device = trio.testing.memory_stream_pair()
            host_write_stream, host_read_stream = breakStapledIntoWriteRead(host)
            dev_write_stream, dev_read_stream = breakStapledIntoWriteRead(device)
            got_to_cancel = False
            with trio.move_on_after(1) as cancel_scope:
                async with trio.open_nursery() as nursery:
                    device = Device(
                        nursery,
                        dev_read_stream,
                        dev_write_stream,
                        nRegisterBits,
                        nRegisters,
                        devicePrintRegistersInterval,
                    )
                    host = Host(
                        nursery, host_read_stream, host_write_stream, nRegisterBits
                    )
                    for iReg in range(nRegisters):
                        await host.write_register(iReg, 0)
                        read_result = await host.read_register(iReg)
                        self.assertEqual(read_result, 0)
                    for iReg in range(nRegisters):
                        await host.write_register(iReg, iReg)
                    for iReg in range(nRegisters):
                        read_result = await host.read_register(iReg)
                        self.assertEqual(read_result, iReg)
                    for iReg in range(nRegisters):
                        await host.write_register(iReg, 0xFFFFFFFF - iReg)
                    for iReg in range(nRegisters):
                        read_result = await host.read_register(iReg)
                        self.assertEqual(read_result, 0xFFFFFFFF - iReg)
                    got_to_cancel = True
                    cancel_scope.cancel()
            self.assertTrue(got_to_cancel)

        trio.run(run_test, self)


class TestCLI(unittest.TestCase):
    def test_read(self):
        async def run_test(self):
            host_to_device_fifo = trio.Path("host_to_device.fifo")
            device_to_host_fifo = trio.Path("device_to_host.fifo")
            if not await host_to_device_fifo.exists():
                os.mkfifo(host_to_device_fifo)
            if not await device_to_host_fifo.exists():
                os.mkfifo(device_to_host_fifo)
            test_timeout = 5  # seconds
            host_timeout = 1  # seconds
            with trio.move_on_after(test_timeout) as move_on_scope:
                async with trio.open_nursery() as nursery:
                    nursery.start_soon(
                        trio.run_process,
                        [
                            "asciiSerialComCli",
                            "read",
                            str(device_to_host_fifo),
                            "0",
                            "--timeout",
                            "7",
                            "--serial-send",
                            str(host_to_device_fifo),
                        ],
                    )
                    # nursery.start_soon(trio.run_process,["asciiSerialComCli","read","input.txt","0","--timeout",str(host_timeout),"--serial-send",str(host_to_device_fifo)])
                    await trio.sleep(0.1)
                    async with await trio.open_file(
                        host_to_device_fifo, "rb"
                    ) as read_fifo:
                        async with await trio.open_file(
                            device_to_host_fifo, "wb"
                        ) as write_fifo:
                            logging.info("Opened files!")
                            data = await read_fifo.read()
                            logging.info(data)
                            data = await read_fifo.read()
                            logging.info(data)
                            await trio.sleep(0.1)
                            await write_fifo.write(data)
                    #        await write_fifo.write(data)
                    #        await write_fifo.write(data)
                    #        await write_fifo.write(data)
                    #        await write_fifo.write(data)
                    #        await write_fifo.write(data)
            await host_to_device_fifo.unlink()
            await device_to_host_fifo.unlink()

        trio.run(run_test, self)


class TestEcho(unittest.TestCase):
    def setUp(self):
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

    def test_both_scripts(self):
        async def run_test(self):
            test_timeout = 10  # seconds
            with trio.move_on_after(test_timeout) as move_on_scope:
                async with trio.open_nursery() as nursery:
                    nursery.start_soon(
                        trio.run_process,
                        [
                            "python",
                            "echo-server.py",
                            str(self.host_to_device_fifo),
                            str(self.device_to_host_fifo),
                        ],
                    )
                    nursery.start_soon(
                        trio.run_process,
                        [
                            "python",
                            "echo-client.py",
                            str(self.device_to_host_fifo),
                            str(self.host_to_device_fifo),
                        ],
                    )

        trio.run(run_test, self)

    def test_just_server(self):
        async def sender(outfilename):
            print(f"sender: started! with send file name: {outfilename}")
            async with await trio.open_file(outfilename, "wb", buffering=0) as f:
                for i in range(5):
                    data = f"async is confusing {i}\n"
                    data = data.encode()
                    print(f"sender: sending {data!r}")
                    await f.write(data)
                    await f.flush()
                    await trio.sleep(0.1)

        async def receiver(infilename):
            print(f"receiver: started! with receive file name: {infilename}")
            async with await trio.open_file(infilename, "rb", buffering=0) as f:
                async for line in f:
                    print(f"receiver received data: '{line!r}'", flush=True)
            print("receiver: file closed")

        async def run_test(self):
            test_timeout = 10  # seconds
            with trio.move_on_after(test_timeout) as move_on_scope:
                async with trio.open_nursery() as nursery:
                    nursery.start_soon(
                        trio.run_process,
                        [
                            "python",
                            "echo-server.py",
                            str(self.host_to_device_fifo),
                            str(self.device_to_host_fifo),
                        ],
                    )
                    nursery.start_soon(sender, self.host_to_device_fifo)
                    nursery.start_soon(receiver, self.device_to_host_fifo)

        trio.run(run_test, self)

    def test_just_client(self):
        async def echo_server(infilename, outfilename):
            print(
                f"echo_server: started with infilename: {infilename} and outfilename: {outfilename}"
            )
            try:
                async with await trio.open_file(
                    infilename, "rb", buffering=0
                ) as infile:
                    async with await trio.open_file(
                        outfilename, "wb", buffering=0
                    ) as outfile:
                        async for line in infile:
                            print(f"echo_server received data: '{line!r}'", flush=True)
                            await outfile.write(line)
                print(f"echo_server: files closed")
            # FIXME: add discussion of MultiErrors to the tutorial, and use
            # MultiError.catch here. (Not important in this case, but important if the
            # server code uses nurseries internally.)
            except Exception as exc:
                # Unhandled exceptions will propagate into our parent and take
                # down the whole program. If the exception is KeyboardInterrupt,
                # that's what we want, but otherwise maybe not...
                print(f"echo_server: crashed: {exc!r}")

        async def run_test(self):
            test_timeout = 10  # seconds
            with trio.move_on_after(test_timeout) as move_on_scope:
                async with trio.open_nursery() as nursery:
                    nursery.start_soon(
                        trio.run_process,
                        [
                            "python",
                            "echo-client.py",
                            str(self.device_to_host_fifo),
                            str(self.host_to_device_fifo),
                        ],
                    )
                    nursery.start_soon(
                        echo_server, self.host_to_device_fifo, self.device_to_host_fifo
                    )

        trio.run(run_test, self)

    def test_no_subprocess(self):
        async def echo_server(infilename, outfilename):
            print(
                f"echo_server: started with infilename: {infilename} and outfilename: {outfilename}"
            )
            try:
                async with await trio.open_file(
                    infilename, "rb", buffering=0
                ) as infile:
                    async with await trio.open_file(
                        outfilename, "wb", buffering=0
                    ) as outfile:
                        async for line in infile:
                            print(f"echo_server received data: '{line!r}'", flush=True)
                            await outfile.write(line)
                print(f"echo_server: files closed")
            # FIXME: add discussion of MultiErrors to the tutorial, and use
            # MultiError.catch here. (Not important in this case, but important if the
            # server code uses nurseries internally.)
            except Exception as exc:
                # Unhandled exceptions will propagate into our parent and take
                # down the whole program. If the exception is KeyboardInterrupt,
                # that's what we want, but otherwise maybe not...
                print(f"echo_server: crashed: {exc!r}")

        async def sender(outfilename):
            print(f"sender: started! with send file name: {outfilename}")
            async with await trio.open_file(outfilename, "wb", buffering=0) as f:
                for i in range(5):
                    data = f"async is confusing {i}\n"
                    data = data.encode()
                    print(f"sender: sending {data!r}")
                    await f.write(data)
                    await f.flush()
                    await trio.sleep(0.1)

        async def receiver(infilename):
            print(f"receiver: started! with receive file name: {infilename}")
            async with await trio.open_file(infilename, "rb", buffering=0) as f:
                async for line in f:
                    print(f"receiver received data: '{line!r}'", flush=True)
            print("receiver: file closed")

        async def run_test(self):
            test_timeout = 10  # seconds
            with trio.move_on_after(test_timeout) as move_on_scope:
                async with trio.open_nursery() as nursery:
                    nursery.start_soon(sender, self.host_to_device_fifo)
                    nursery.start_soon(receiver, self.device_to_host_fifo)
                    nursery.start_soon(
                        echo_server, self.host_to_device_fifo, self.device_to_host_fifo
                    )

        trio.run(run_test, self)
