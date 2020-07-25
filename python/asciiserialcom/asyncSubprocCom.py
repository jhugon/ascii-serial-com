"""
Handle communication with a subprocess without deadlocking
"""

import threading
import time
import queue
import os
import subprocess


class FileReaderThread(threading.Thread):
    """
    Used to read from a file without blocking the main thread
    In particular for reading from pipes and avoiding deadlocks

    Assumes data is bytes

    Uses a queue.Queue to buffer and pass data between threads

    Usage:

    1) Initialize with the file obj to be read from
    2) call the start() method to start reading
    3) periodically call the get_data() method to get the read data

    """

    def __init__(self, file_obj):
        """
        file_obj is a file object open for reading
        """
        super().__init__(daemon=True)
        self.file_obj = file_obj
        self.q = queue.Queue()

    def run(self):
        """
        The code that will be run in another thread
        """
        while True:
            # both put and read can block
            try:
                # print("FileReaderThread: about to call file_obj.read")
                data = self.file_obj.read(64)
            except ValueError as e:
                # print("ValueError: ", e, flush=True)
                return
            # print(
            #    "FileReaderThread: actually read something of length {}: '{}'".format(
            #        len(data), data
            #    ),
            #    flush=True,
            # )
            self.q.put(data)
            # print(
            #    "FileReaderThread: put the data in q, qsize: {}".format(self.q.qsize()),
            #    flush=True,
            # )

    def receive(self):
        """
        Call to get data read in the other thread
        Will keep getting the data until the queue is empty
        If there is no data in queue, then returns empty bytearray
        """
        result = bytearray()
        try:
            while True:
                result += self.q.get(block=False, timeout=0.02)
        except queue.Empty:
            pass
        # print("get_data returning: {}".format(result), flush=True)
        return result


class Async_Subproc_Com(object):
    """
    Handle communication with a subprocess without deadlocking
    """

    def __init__(self, procargslist, env=None):
        self.proc = subprocess.Popen(
            procargslist,
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            bufsize=0,
            close_fds=True,
        )
        self.frt = FileReaderThread(self.proc.stdout)
        # self.fwt.start()
        self.frt.start()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, value, traceback):
        self.proc.stdout.close()
        time.sleep(1e-2)
        self.proc.__exit__(exc_type, value, traceback)

    def send(self, data):
        self.proc.stdin.write(data)

    def receive(self):
        return self.frt.receive()
