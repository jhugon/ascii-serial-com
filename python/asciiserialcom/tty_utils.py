"""
Functions to setup TTYs
"""

import inspect
import termios
import tty
import sys


def setup_tty(f, speed):
    """
    Setup the TTY for use with ASCII Serial Com.

    The TTY is set to raw mode and VMIN is set to 0. This seems
    to be what the Arduino serial monitor does.

    f: file object or file descriptor (int)

    speed: an int baud rate. Must be a supported value by termios
        If you use an unsupported value, will print out the
        available options and raise Exception
    """
    try:
        speedconstname = "B{:d}".format(speed)
        speedconst = getattr(termios, speedconstname)
    except AttributeError:
        print(f"Error: setup_tty: speed not supported: {speed}", file=sys.stderr)
        members = [x[0] for x in inspect.getmembers(termios)]
        avail_speeds = [
            x[1:] for x in members if len(x) > 2 and x[0] == "B" and x[1].isdecimal()
        ]
        avail_speeds.sort(key=int)
        print("Available options:", ", ".join(avail_speeds))
        raise Exception
    else:
        tty.setraw(f)
        tty_attrs = termios.tcgetattr(f)
        tty_attrs[6][termios.VMIN] = 0
        tty_attrs[4] = speedconst
        tty_attrs[5] = speedconst
        termios.tcsetattr(f, termios.TCSAFLUSH, tty_attrs)
