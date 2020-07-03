"""
Shell for interacting through ASCII Serial Com
"""

import sys
import os.path
import math
import cmd
import argparse
from asciiserialcom.asciiSerialCom import Ascii_Serial_Com
from asciiserialcom.ascErrors import *


class CmdArgumentParser(argparse.ArgumentParser):
    """
    Argument parser for cmd.Cmd shells
    """

    def exit(self, status=0, message=None):
        if message is not None:
            print("Error:", message)
        raise ShellArgumentError(f"Command error: {message}")

    def error(self, message=None):
        if message is not None:
            print("Error:", message)
        raise ShellArgumentError(f"Command error: {message}")


class Ascii_Serial_Com_Shell(cmd.Cmd):
    intro = "ASCII Serial Com Shell.   Type help or ? to list commands.\n"
    prompt = "(ASCII Serial Com) "
    file = None

    def __init__(self, fin, fout, registerBitWidth):
        self.asc = Ascii_Serial_Com(fin, fout, registerBitWidth, crcFailBehavior="warn")
        super().__init__()

    def _to_int(self, s):
        """
        Converts string to int
        """

        if s[:2].lower() == "0x":
            return int(s[2:], 16)
        elif s[:2].lower() == "0b":
            return int(s[2:], 2)
        else:
            return int(s, 10)

    # ----- commands -----
    def do_w(self, arg):
        "w <reg_num> <reg_val>\n\nWrite reg_val to reg_num. Assumes both are base-10 ints, unless 0x is prefix, then hex."
        parser = CmdArgumentParser(
            prog="w", description=Ascii_Serial_Com_Shell.do_w.__doc__
        )
        parser.add_argument("reg_num", help="Register number")
        parser.add_argument("reg_val", help="Register value to write")

        try:
            args = parser.parse_args(arg.split())
        except ShellArgumentError:
            return
        reg_num = self._to_int(args.reg_num)
        reg_val = self._to_int(args.reg_val)
        try:
            self.asc.write_register(reg_num, reg_val)
        except ASCErrorBase as e:
            print(e.args)
            return

    def do_r(self, arg):
        "r <reg_num>\n\nReads register reg_num. Assumes reg_num is a base-10 int, unless 0x is prefix, then hex."

        parser = CmdArgumentParser(
            prog="r", description=Ascii_Serial_Com_Shell.do_r.__doc__
        )
        parser.add_argument("reg_num", help="Register number")

        try:
            args = parser.parse_args(arg.split())
        except ShellArgumentError:
            return
        reg_num = self._to_int(args.reg_num)
        try:
            result = self.asc.read_register(reg_num)
        except ASCErrorBase as e:
            print(e.args)
            return
        else:
            bitWidth = self.asc.getRegisterBitWidth()
            hexWidth = str(int(math.ceil(bitWidth / 4)))
            decWidth = str(int(math.ceil(bitWidth * math.log10(2))))
            bitWidth = str(bitWidth)
            formatstr = (
                "{0:"
                + decWidth
                + "d} = 0x{0:0"
                + hexWidth
                + "X} = {0:#0"
                + bitWidth
                + "b}"
            )
            print(formatstr.format(int(result, 16)))

    def do_send(self, arg):
        "send <command> <data>\n\nSends message to device. Command should be a single letter. Data is directly sent as ASCII bytes."

        parser = CmdArgumentParser(
            prog="send", description=Ascii_Serial_Com_Shell.do_send.__doc__
        )
        parser.add_argument("command", help="Command to send (a single letter)")
        parser.add_argument("data", help="Data to send (ASCII bytes)")

        try:
            args = parser.parse_args(arg.split())
        except ShellArgumentError:
            return
        try:
            self.asc.send_message(args.command, args.data)
        except ASCErrorBase as e:
            print(e.args)
            return

    def do_EOF(self, arg):
        print("")  # for newline
        sys.exit(0)

    def do_exit(self, arg):
        sys.exit(0)

    def do_quit(self, arg):
        sys.exit(0)


def main():
    parser = argparse.ArgumentParser(
        description="ASCII Serial Com Shell. Useful for debugging ASCII Serial Com connections."
    )
    parser.add_argument("fin", help="Path to tty device file")
    parser.add_argument("fout", help="Path to tty device file")
    parser.add_argument(
        "--registerBitWidth",
        "-r",
        type=int,
        default=32,
        help="Device register bit width (default: 32)",
    )

    args = parser.parse_args()

    inFname = os.path.abspath(args.fin)
    outFname = os.path.abspath(args.fout)
    print(f"Input file name: {inFname}")
    print(f"outFname file name: {outFname}")
    print(f"Register bits: {args.registerBitWidth}")

    with open(outFname, "wb", buffering=0) as fout:
        with open(inFname, "rb", buffering=0) as fin:
            Ascii_Serial_Com_Shell(fin, fout, args.registerBitWidth).cmdloop()
