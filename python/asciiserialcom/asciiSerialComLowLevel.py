"""
ASCII Serial Com Python Interface
"""

from .ascErrors import *
from .circularBuffer import Circular_Buffer_Bytes
from .ascMessage import ASC_Message


async def send_message(fout, asciiSerialComVersion, appVersion, command, data):
    """
    Low-level message send command
    Does not check if command is defined command

    command: single byte lower-case letter command/message type
    data: bytes or None

    returns None
    """
    msg = ASC_Message(asciiSerialComVersion, appVersion, command, data)
    message = msg.get_packed()
    print(
        "send_message: command: {} data: {} message: {}".format(command, data, message)
    )
    await fout.write(message)
    await fout.flush()


async def receiver_loop(
    fin, queue_w, queue_r, queue_s, asciiSerialComVersion, appVersion
):
    """
    This is the task that handles reading from the serial link with file like object fin
    and then puts ASC_Message's in the queues
    """
    buf = Circular_Buffer_Bytes(128)
    while True:
        msg = await receive_message(fin, buf, asciiSerialComVersion, appVersion)
        if msg:
            if msg.command == b"w":
                queue_w.put(msg)
            elif msg.command == b"r":
                queue_r.put(msg)
            elif msg.command == b"s":
                queue_s.put(msg)
            else:
                pass


async def receive_message(fin, buf, asciiSerialComVersion, appVersion):
    """
    fin: file-like object to read from
    buf: circular buffer

    returns a ASC_Message

    if no frame is received, all members ASC_Message will be None

    """
    frame = await frame_from_stream(fin, buf)
    if frame is None:
        return ASC_Message(None, None, None, None)
    print("received message: {}".format(frame))
    msg = ASC_Message.unpack(frame)
    if msg.ascVersion != asciiSerialComVersion:
        raise AsciiSerialComVersionMismatchError(
            "Message version: {} Expected version: {}".format(
                msg.ascVersion, asciiSerialComVersion
            )
        )
    if msg.appVersion != appVersion:
        raise ApplicationVersionMismatchError(
            "Message version: {} Expected version: {}".format(
                msg.appVersion, appVersion
            )
        )
    return msg


async def frame_from_stream(fin, buf):
    """
    Reads bytes from file-like object and attempts to identify a message frame. Uses circular_buffer buf.

    returns: frame as bytes; None if no frame found in stream
    """
    try:
        b = await fin.read()
    except ValueError:
        raise FileReadError
    except IOError:
        raise FileReadError
    else:
        buf.push_back(b)
        buf.removeFrontTo(b">", inclusive=False)
        if len(buf) == 0:
            return None
        iNewline = buf.findFirst(b"\n")
        if iNewline is None:
            return None
        return buf.pop_front(iNewline + 1)
