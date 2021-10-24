"""
Handle communication with a subprocess without deadlocking
"""

import threading
import time
import queue
import os
import subprocess


class Com_Subproc(object):
    """
    Handle communication with a subprocess without deadlocking

    Receives subproc output in another thread which puts it in a queue
    """

    def __init__(self, procargslist, env=None, hideStderr=False):
        """
        procargslist is a list of args to pass to Popen

        env is a dict of the environment to pass to Popen
        """
        stderr = None
        if hideStderr:
            stderr = subprocess.DEVNULL
        self.proc = subprocess.Popen(
            procargslist,
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=stderr,
            bufsize=0,
            close_fds=True,
        )
        self.frt = FileReaderThread(self.proc.stdout)
        # self.fwt.start()
        self.frt.start()

    def send(self, data):
        """Write to stdin of the subprocess"""
        self.proc.stdin.write(data)

    def receive(self):
        """
        Read from stdout of the subprocess, with a 20 ms timout.

        Returns empty bytearray on timeout
        """
        return self.frt.receive()

    def terminate(self):
        self.proc.terminate()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, value, traceback):
        self.proc.stdout.close()
        time.sleep(1e-2)
        self.proc.__exit__(exc_type, value, traceback)


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
            if len(data) > 0:
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
            else:
                time.sleep(5e-3)

    def receive(self):
        """
        Call to get data read in the other thread
        Will keep getting the data until the queue is empty
        If there is no data in queue, then returns empty bytearray
        """
        result = bytearray()
        try:
            while True:
                # result += self.q.get(block=False, timeout=0.001)
                result += self.q.get_nowait()
        except queue.Empty:
            pass
        # if len(result) > 0:
        #     print("receive returning: {}".format(result), flush=True)
        return result
