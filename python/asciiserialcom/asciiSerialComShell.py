"""
Shell for interacting through ASCII Serial Com
"""

import sys
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

    def __init__(self, f, registerBitWidth):
        self.asc = Ascii_Serial_Com(f, registerBitWidth, crcFailBehavior="warn")
        super().__init__()

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
        try:
            self.asc.write_register(args.reg_num, args.reg_val)
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
        try:
            result = self.asc.read_register(args.reg_num)
        except ASCErrorBase as e:
            print(e.args)
            return
        else:
            bitWidth = self.asc.getRegisterBitWidth()
            hexWidth = str(int(math.ceil(bitWidth / 4)))
            decWidth = str(int(math.ceil(bitWidth * math.log10(2))))
            bitWidth = str(bitWidth)
            formatstr = (
                "{0:0"
                + decWidth
                + "d} = 0x{0:0"
                + hexWidth
                + "X} = {0:#0"
                + bitWidth
                + "b}"
            )
            print(formatstr.format(result))

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

    def do_exit(self, arg):
        sys.exit(0)

    def do_quit(self, arg):
        sys.exit(0)


def main():
    parser = argparse.ArgumentParser(
        description="ASCII Serial Com Shell. Useful for debugging ASCII Serial Com connections."
    )
    parser.add_argument("tty", help="Path to tty device file")
    parser.add_argument(
        "--registerBitWidth",
        "-r",
        type=int,
        default=32,
        help="Device register bit width (default: 32)",
    )

    args = parser.parse_args()

    with open(args.tty, "r+b") as tty:
        Ascii_Serial_Com_Shell(tty, args.registerBitWidth).cmdloop()