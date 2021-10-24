#!/usr/bin/env python3
"""
Shell for interacting through ASCII Serial Com
"""

import subprocess

import inspect
import termios
import tty
import sys


def setup_tty(f, speed):
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
        tty_attrs[6][termios.VMIN] = 1
        tty_attrs[4] = speedconst
        tty_attrs[5] = speedconst
        termios.tcsetattr(f, termios.TCSAFLUSH, tty_attrs)


def pprint(b):
    result = b.decode()
    result = result.replace("\n", " ")
    print(result)


def main():

    all_members = inspect.getmembers(termios)
    int_members = [x for x in all_members if type(x[1]) is int]
    for var in int_members:
        if var[0][:2] == "TC":
            continue
        print(var)

    # subprocess.run(["stty","-F","/dev/ttyACM0","sane"])

    out_good = subprocess.run(["stty", "-F", "/dev/ttyACM0"], capture_output=True)
    out_good_all = subprocess.run(
        ["stty", "-F", "/dev/ttyACM0", "-a"], capture_output=True
    )

    good_attrs = None
    raw_attrs = None

    fname = "/dev/ttyACM0"
    with open("/dev/ttyACM0", "rb") as f:
        good_attrs = termios.tcgetattr(f)
        (iflag, oflag, cflag, lflag, ispeed, ospeed, cc) = good_attrs
        #        tty.setraw(f)
        setup_tty(f, 50)
        raw_attrs = termios.tcgetattr(f)

    out_raw = subprocess.run(["stty", "-F", "/dev/ttyACM0"], capture_output=True)
    out_raw_all = subprocess.run(
        ["stty", "-F", "/dev/ttyACM0", "-a"], capture_output=True
    )

    with open("/dev/ttyACM0", "rb") as f:
        termios.tcsetattr(f, termios.TCSAFLUSH, good_attrs)

    out_good_check = subprocess.run(["stty", "-F", "/dev/ttyACM0"], capture_output=True)
    out_good_all_check = subprocess.run(
        ["stty", "-F", "/dev/ttyACM0", "-a"], capture_output=True
    )
    if out_good.stdout == out_good_check.stdout:
        print("Check: options match!")
    else:
        print("Check: options don't match!")
        print(out_good.stdout)
        print(out_good_check.stdout)
    if out_good_all.stdout == out_good_all_check.stdout:
        print("Check: all options match!")
    else:
        print("Check: all options don't match!")
        print(out_good_all.stdout)
        print(out_good_all_check.stdout)

    print("Good then raw options from stty:")
    pprint(out_good.stdout)
    pprint(out_raw.stdout)
    print("Good then raw options from stty -g:")
    pprint(out_good_all.stdout)
    pprint(out_raw_all.stdout)
    print("Good then raw termios attrs:")
    print(good_attrs)
    print(raw_attrs)


if __name__ == "__main__":
    main()
