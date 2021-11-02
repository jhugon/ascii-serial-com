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

logging.basicConfig(
    # filename="test_asciiSerialCom.log",
    # level=logging.INFO,
    level=logging.DEBUG,
    # format="%(asctime)s %(levelname)s L%(lineno)d %(funcName)s: %(message)s"
    format="%(message)s",
)


class TestRxFromDevice(unittest.TestCase):
    """
    Requires firmware:

        avrdude -p atmega328p -c arduino -P /dev/ttyACM0 -Uflash:w:build/avr5_gcc_debug/arduino_uno_write_pattern_to_serial

    """

    @unittest.skip(
        "Doesn't work reliably. Need pySerial help. I don't think Trio keeps up with input well."
    )
    def test_just_device(self):
        async def receiver(chan, f):
            while True:
                data = await f.read(10)
                await chan.send(data)

        async def data_checker(self, chan, chan_all_done):
            last = None
            print("Starting data_checker", flush=True)
            try:
                for i in range(200):
                    if i > 2:
                        data = await chan.receive()
                        print(f"Receive: {i}")
                        for ch in data:
                            await trio.sleep(0)
                            num = ch - 48
                            print(num, flush=True)
                            if last == 9 and num != 0:
                                raise ValueError()
                            elif last and num != last + 1:
                                raise ValueError()
                            last = num
            except ValueError:
                self.assertTrue(False)
            await chan_all_done.send("All done!")

        async def run_test(self):
            logging.info("Starting run_test")
            send_chan, recv_chan = trio.open_memory_channel(100)
            send_chan_all_done, recv_chan_all_done = trio.open_memory_channel(0)
            got_to_cancel = False
            with trio.move_on_after(10) as cancel_scope:
                logging.info("About to open file...")
                async with await trio.open_file("/dev/ttyACM0", "br") as portr:
                    setup_tty(portr.wrapped, 9600)
                    async with trio.open_nursery() as nursery:
                        nursery.start_soon(receiver, send_chan, portr)
                        nursery.start_soon(
                            data_checker, self, recv_chan, send_chan_all_done
                        )
                        print(await recv_chan_all_done.receive())
                        got_to_cancel = True
                        cancel_scope.cancel()
            self.assertTrue(got_to_cancel)

        trio.run(run_test, self)

    def test_just_device_pyserial(self):
        # print("Starting")
        with serial.Serial("/dev/ttyACM0", 9600, 8, "N", 1) as ser:
            # print(ser)
            last = None
            ser.reset_input_buffer()
            for i in range(5):
                ser.read(100)
            for i in range(2000):
                data = ser.read(1)
                num = int(data)
                # print(f"{i:5} {num:2}")
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
                await chan.send(data)

        async def data_checker(self, chan, chan_all_done):
            last = None
            for i in range(2000):
                if i > 2:
                    data = await chan.receive()
                    num = int(data)
                    if last == 9:
                        self.assertEqual(num, 0)
                    elif last:
                        self.assertEqual(num, last + 1)
                    last = num
            await chan_all_done.send("All done!")

        async def run_test(self):
            # logging.info("Starting run_test")
            with serial.Serial("/dev/ttyACM0", 9600, 8, "N", 1) as ser:
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


class TestCharLoopback(unittest.TestCase):
    """
        Requires firmware:

        avrdude -p atmega328p -c arduino -P /dev/ttyACM0 -Uflash:w:build/avr5_gcc_debug/arduino_uno_char_loopback

    """

    def test_just_device(self):
        async def reader(self, ser, task_status=trio.TASK_STATUS_IGNORED):
            with trio.move_on_after(5) as moveon_scope:
                async with trio.wrap_file(ser) as port:
                    task_status.started()
                    try:
                        # while True:
                        for i in range(6):
                            logging.debug(f"About to read data {i}")
                            data = await port.read(1)
                            logging.debug("Done reading data")
                            # print(data.decode(),flush=True,end="")
                            print(data.decode(), flush=True)
                            logging.debug("Done printing read data")
                    except Exception as e:
                        logging.error(e)
                        print("Reader Exception!!!", flush=True)
            print("Reader exiting", flush=True)

        async def run_test(self):
            with trio.move_on_after(1) as moveon_scope:
                with serial.Serial("/dev/ttyACM0", 9600, 8, "N", 1) as ser:
                    async_ser = trio.wrap_file(ser)
                    async with trio.open_nursery() as nursery:
                        # nursery.start_soon(reader,self,async_ser)
                        await nursery.start(reader, self, ser)
                        await trio.sleep(1)
                        await async_ser.write(b"c")
                        print("Write c")
                        await trio.sleep(1)
                        await async_ser.write(b"d")
                        print("Write d")
                        await async_ser.write(b"e")
                        print("Write e")
                        await async_ser.write(b"f")
                        print("Write f")
                        await trio.sleep(1)
                        print("About to cancel")
                        nursery.cancel_scope.cancel()
                        print("About to cancel with moveon")
                        moveon_scope.cancel()

        trio.run(run_test, self, instruments=[Tracer()])
        return

        import threading

        ser = None

        def receiver():
            try:
                while True:
                    data = ser.read(1)
                    print(f"Received: {data}")
            except BaseException as e:
                print("Error:")
                print(e)

        with serial.Serial("/dev/ttyACM0", 9600, 8, "N", 1) as ser:
            print(ser.read(1))
