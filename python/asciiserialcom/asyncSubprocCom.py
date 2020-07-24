"""
Handle communication with a subprocess without deadlocking
"""

import threading
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
                print("FileReaderThread: about to call file_obj.read")
                data = self.file_obj.read(64)
            except ValueError as e:
                print("ValueError: ", e, flush=True)
                return
            print(
                "FileReaderThread: actually read something of length {}: '{}'".format(
                    len(data), data
                ),
                flush=True,
            )
            self.q.put(data)
            print(
                "FileReaderThread: put the data in q, qsize: {}".format(self.q.qsize()),
                flush=True,
            )

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


class FileWriterThread(threading.Thread):
    def __init__(self, file_obj):
        super().__init__(daemon=True)
        self.file_obj = file_obj
        self.q = queue.Queue()

    def run(self):
        while True:
            print("FileWriterThread: about to call q.get")
            data = self.q.get()
            print("FileWriterThread: got some data: ", data)
            self.file_obj.write(data)
            print("FileWriterThread: wrote the data")

    def send(self, data):
        self.q.put(data)
        print("FileWriterThread: put some data in q")


class Async_Subproc_Com(object):
    """
    Handle communication with a subprocess without deadlocking
    """

    def __init__(self, procargslist, env=None):
        self.p1r, self.p1w = os.pipe()
        self.p2r, self.p2w = os.pipe()

        self.f1r = os.fdopen(self.p1r, "rb")
        self.f1w = os.fdopen(self.p1w, "wb")
        self.f2r = os.fdopen(self.p2r, "rb")
        self.f2w = os.fdopen(self.p2w, "wb")

        self.fwt = FileWriterThread(self.f1w)
        self.frt = FileReaderThread(self.f2r)
        self.fwt.start()
        self.frt.start()

        self.proc = subprocess.Popen(
            procargslist, env=env, stdin=self.f1r, stdout=self.f2w, bufsize=0
        )

    def __del__(self):
        self.proc.terminate()
        self.f1r.close()
        self.f1w.close()
        self.f2r.close()
        self.f2w.close()

    def send(self, data):
        # self.fwt.send(data)
        self.f1w.write(data)

    def receive(self):
        return self.fwt.receive(data)


if __name__ == "__main__":
    f = None
    asc = Ascii_Serial_Com(f, 32)
    print(asc)
