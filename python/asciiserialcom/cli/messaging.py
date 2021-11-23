from typing import Optional
from pathlib import Path
import logging
from functools import partial

import typer
import trio
import trio_util  # type: ignore

from ..host import Host
from ..utilities import Tracer
from ..tty_utils import setup_tty
from ..errors import *

DEFAULT_TIMEOUT = 5

SERIAL = None
SERIAL_SEND = None
VERBOSE = False


async def run_read(timeout, reg_num):
    fin_name = SERIAL
    fout_name = SERIAL
    if SERIAL_SEND:
        fout_name = SERIAL_SEND
    logging.debug(f"fin_name: {fin_name}")
    logging.debug(f"fout_name: {fout_name}")
    result = None
    with trio.move_on_after(timeout) as cancel_scope:
        async with trio.open_nursery() as nursery:
            async with await trio.open_file(fin_name, "br") as fin:
                if fin_name.is_char_device():
                    setup_tty(fin.wrapped, 9600)
                async with await trio.open_file(fout_name, "bw") as fout:
                    logging.debug("Opened files and nursery")
                    host = Host(nursery, fin, fout, 8, ignoreErrors=True)
                    # await host.stop_streaming()
                    result = await host.read_register(reg_num)
    return result


async def run_write(timeout, reg_num, reg_val):
    fin_name = SERIAL
    fout_name = SERIAL
    if SERIAL_SEND:
        fout_name = SERIAL_SEND
    logging.debug(f"fin_name: {fin_name}")
    logging.debug(f"fout_name: {fout_name}")
    result = False
    with trio.move_on_after(timeout) as cancel_scope:
        async with trio.open_nursery() as nursery:
            async with await trio.open_file(fin_name, "br") as fin:
                if fin_name.is_char_device():
                    setup_tty(fin.wrapped, 9600)
                async with await trio.open_file(fout_name, "bw") as fout:
                    logging.debug("Opened files and nursery")
                    host = Host(nursery, fin, fout, 8, ignoreErrors=True)
                    # await host.stop_streaming()
                    await host.write_register(reg_num, reg_val)
                result = True
    return result


async def forward_received_messages_to_print(
    ch,
    outfile,
    stop_messages,
    stop_bytes,
    stop_seperators,
    stop_event,
    split_seperators_newlines,
    decode_hex_to_dec,
):
    logging.debug("Started")
    totalMessages = 0
    totalBytes = 0
    totalSeperators = 0
    totalMissedMessages = 0
    f = None
    try:
        if outfile:
            f = await trio.open_file(outfile, "w")
        while True:
            try:
                nMissed, payload = await ch.receive()
            except trio.EndOfChannel:
                break
            else:
                out_text = ""
                totalMessages += 1
                totalBytes += len(payload)
                totalSeperators += payload.count(b" ")
                msg_text = payload.decode("ascii", "replace")
                if split_seperators_newlines or decode_hex_to_dec:
                    for x in msg_text.split(" "):
                        if decode_hex_to_dec:
                            out_text += str(int(msg_text, 16)) + "\n"
                        else:
                            out_text += msg_text + "\n"
                else:
                    out_text += msg_text + "\n"
                if f:
                    await f.write(out_text)
                else:
                    typer.echo(out_text, nl=False)
                if stop_messages and totalMessages >= stop_messages:
                    stop_event.set()
                    break
                elif stop_bytes and totalBytes >= stop_bytes:
                    stop_event.set()
                    break
                elif stop_seperators and totalSeperators >= stop_seperators:
                    stop_event.set()
                    break
            finally:
                if totalMissedMessages > 0:
                    logging.warning(
                        f"Receiver missed a total of {totalMissedMessages} messages"
                    )
    except Exception as e:
        raise e
    finally:
        if f:
            await f.aclose()


async def run_stream(
    timeout,
    outfile,
    stop_messages,
    stop_bytes,
    stop_seperators,
    split_seperators_newlines,
    decode_hex_to_dec,
):
    fin_name = SERIAL
    fout_name = SERIAL
    if SERIAL_SEND:
        fout_name = SERIAL_SEND
    logging.debug(f"fin_name: {fin_name}")
    logging.debug(f"fout_name: {fout_name}")
    logging.debug(f"timeout: {timeout}")
    send_ch, recv_ch = trio.open_memory_channel(0)
    if timeout is None:
        timeout = float("inf")
    timeout_cancel = timeout + 0.5
    with trio.move_on_after(timeout_cancel) as cancel_scope:
        async with trio.open_nursery() as nursery:
            logging.debug(f"About to open files")
            async with await trio.open_file(fin_name, "br") as fin:
                async with await trio.open_file(fout_name, "bw") as fout:
                    if fin_name.is_char_device():
                        setup_tty(fin.wrapped, 9600)
                    logging.debug(f"Files open!")
                    host = Host(nursery, fin, fout, 8, ignoreErrors=True)
                    stop_event = trio.Event()
                    nursery.start_soon(
                        forward_received_messages_to_print,
                        recv_ch,
                        outfile,
                        stop_messages,
                        stop_bytes,
                        stop_seperators,
                        stop_event,
                        split_seperators_newlines,
                        decode_hex_to_dec,
                    )
                    host.forward_received_s_messages_to(send_ch)
                    await host.start_streaming()
                    await trio_util.wait_any(
                        partial(trio.sleep, timeout), stop_event.wait
                    )
                    await host.stop_streaming()


app = typer.Typer()


@app.callback()
def callback(
    serial: Path = typer.Argument(
        ...,
        help="Filename of the serial device",
        exists=True,
        writable=True,
        readable=True,
    ),
    serial_send: Optional[Path] = typer.Option(
        None,
        help="Filename of the serial device to write to. If present, SERIAL is only read from",
        exists=True,
        writable=True,
    ),
    verbose: bool = typer.Option(False, "--verbose", "-v"),
) -> None:
    """
    Communicate with a device with ASCII-Serial-Com

    There are two levels of options: those before the serial device and those after. They are seperate groups.
    """
    global SERIAL
    global SERIAL_SEND
    global VERBOSE
    SERIAL = serial
    SERIAL_SEND = serial_send
    VERBOSE = verbose
    logging.debug(f"SERIAL: {SERIAL}, SERIAL_SEND: {SERIAL_SEND}, VERBOSE: {VERBOSE}")


@app.command()
def read(
    register_number: int = typer.Argument(..., help="Register number", min=0),
    timeout: float = typer.Option(
        DEFAULT_TIMEOUT, help="Timeout for register read", min=0.0
    ),
) -> None:
    if SERIAL_SEND:
        typer.echo(
            f"Reading register {register_number} on send device {SERIAL_SEND} and read device {SERIAL}"
        )
    else:
        typer.echo(f"Reading register {register_number} on device {SERIAL}")
    try:
        result = trio.run(run_read, timeout, register_number)
        if result is None:
            typer.echo(
                f"Error: read reply not received and timeout expired after {timeout} s",
                err=True,
            )
        else:
            typer.echo(
                f'Register {register_number} value is {result:5} = 0x{result:02x} = UTF-8: "{chr(result)}"'
            )
    except Exception as e:
        typer.echo(f"Error: unhandled exception: {type(e)}: {e}", err=True)
        raise e


@app.command()
def write(
    register_number: int = typer.Argument(..., help="Register number", min=0),
    register_value: int = typer.Argument(
        ..., help="Register value to write to device", min=0
    ),
    timeout: float = typer.Option(
        DEFAULT_TIMEOUT, help="Timeout for register read", min=0.0
    ),
) -> None:
    if SERIAL_SEND:
        typer.echo(
            f"Writing {register_value} (0x{register_value:02x}) to register {register_number} on send device {SERIAL_SEND} and read device {SERIAL}"
        )
    else:
        typer.echo(
            f"Writing {register_value} (0x{register_value:02x}) to register {register_number} on device {SERIAL}"
        )
    try:
        result = trio.run(run_write, timeout, register_number, register_value)
        if result:
            typer.echo(f"Success")
        else:
            typer.echo(
                f"Error: write reply not received and timeout expired after {timeout} s",
                err=True,
            )
    except Exception as e:
        typer.echo(f"Error: unhandled exception: {type(e)}: {e}", err=True)


@app.command()
def stream(
    outfile: Optional[Path] = typer.Option(
        None,
        help="Filename to write received data to instead of printing to stdout",
        writable=True,
    ),
    stop_seconds: Optional[float] = typer.Option(
        None, help="Stop after this many seconds"
    ),
    stop_messages: Optional[int] = typer.Option(
        None, help="Stop after this many messages have been received"
    ),
    stop_bytes: Optional[int] = typer.Option(
        None, help="Stop after this many bytes have been received"
    ),
    stop_datasep: Optional[int] = typer.Option(
        None,
        help="Stop after this many data-seperater characters have been received (spaces in the data field)",
    ),
    split_seperators_newlines: Optional[bool] = typer.Option(
        False,
        help="In addition to a newline for every message, there is a newline for every seperator",
    ),
    decode_hex_to_dec: Optional[bool] = typer.Option(
        False,
        help="Convert each message (or seperated chunk of data) from hexadecimal to decimal integer. Implies --split-seperators-newlines",
    ),
) -> None:
    """
    Either prints message data to the screen, one message per line, or writes the same to OUTFILE

    With all the stop arguments and hitting Ctrl-C: whichever happens first will stop streaming.
    """
    if SERIAL_SEND:
        typer.echo(
            f"Receive streaming data with send device {SERIAL_SEND} and read device {SERIAL}"
        )
    else:
        typer.echo(f"Receive streaming data with device {SERIAL}")
    try:
        trio.run(
            run_stream,
            stop_seconds,
            outfile,
            stop_messages,
            stop_bytes,
            stop_datasep,
            split_seperators_newlines,
            decode_hex_to_dec,
        )  # ,instruments=[Tracer()])
    except Exception as e:
        typer.echo(f"Error: unhandled exception: {type(e)}: {e}", err=True)
        raise e


def main():
    logging.basicConfig(
        # filename="test_hostiiSerialCom.log",
        # level=logging.INFO,
        # level=logging.DEBUG,
        format="%(levelname)s L%(lineno)d %(funcName)s: %(message)s",
    )

    app()
