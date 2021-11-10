from typing import Optional
from pathlib import Path
import logging
from functools import partial

import typer
import trio
import trio_util  # type: ignore

from .host import Host
from .utilities import Tracer

DEFAULT_TIMEOUT = 5


async def run_read(timeout, fin_name, fout_name, reg_num):
    logging.debug(f"fin_name: {fin_name}")
    logging.debug(f"fout_name: {fout_name}")
    result = None
    with trio.move_on_after(timeout) as cancel_scope:
        async with trio.open_nursery() as nursery:
            async with await trio.open_file(fin_name, "br") as fin:
                async with await trio.open_file(fout_name, "bw") as fout:
                    logging.debug("Opened files and nursery")
                    host = Host(nursery, fin, fout, 8)
                    result = await host.read_register(reg_num)
    return result


async def run_write(timeout, fin_name, fout_name, reg_num, reg_val):
    logging.debug(f"fin_name: {fin_name}")
    logging.debug(f"fout_name: {fout_name}")
    result = False
    with trio.move_on_after(timeout) as cancel_scope:
        async with trio.open_nursery() as nursery:
            async with await trio.open_file(fin_name, "br") as fin:
                async with await trio.open_file(fout_name, "bw") as fout:
                    logging.debug("Opened files and nursery")
                    host = Host(nursery, fin, fout, 8)
                    await host.write_register(reg_num, reg_val)
                result = True
    return result


async def forward_received_messages_to_print(
    ch, stop_messages, stop_bytes, stop_seperators, stop_event
):
    logging.debug("Started")
    totalMessages = 0
    totalBytes = 0
    totalSeperators = 0
    while True:
        try:
            msg = await ch.receive()
        except trio.EndOfChannel:
            break
        else:
            typer.echo(f"{msg.data.decode('ascii','replace')}")
            totalMessages += 1
            totalBytes += len(msg.data)
            totalSeperators += msg.data.count(b" ")
            if stop_messages and totalMessages >= stop_messages:
                stop_event.set()
                break
            elif stop_bytes and totalBytes >= stop_bytes:
                stop_event.set()
                break
            elif stop_seperators and totalSeperators >= stop_seperators:
                stop_event.set()
                break


async def run_stream(
    timeout, stop_messages, stop_bytes, stop_seperators, fin_name, fout_name
):
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
                    logging.debug(f"Files open!")
                    host = Host(nursery, fin, fout, 8)
                    stop_event = trio.Event()
                    nursery.start_soon(
                        forward_received_messages_to_print,
                        recv_ch,
                        stop_messages,
                        stop_bytes,
                        stop_seperators,
                        stop_event,
                    )
                    host.forward_received_s_messages_to(send_ch)
                    await host.send_message(b"n", b"")
                    await trio_util.wait_any(
                        partial(trio.sleep, timeout), stop_event.wait
                    )
                    await host.send_message(b"f", b"")


app = typer.Typer()


@app.command()
def read(
    serial: Path = typer.Argument(
        ...,
        help="Filename of the serial device",
        exists=True,
        writable=True,
        readable=True,
    ),
    register_number: int = typer.Argument(..., help="Register number", min=0),
    timeout: float = typer.Option(
        DEFAULT_TIMEOUT, help="Timeout for register read", min=0.0
    ),
    serial_send: Optional[Path] = typer.Option(
        None,
        help="Filename of the serial device to write to. If present, SERIAL is only read from",
        exists=True,
        writable=True,
    ),
) -> None:
    if serial_send:
        typer.echo(
            f"Reading register {register_number} on send device {serial_send} and read device {serial}"
        )
    else:
        typer.echo(f"Reading register {register_number} on device {serial}")
        serial_send = serial
    try:
        result = trio.run(run_read, timeout, serial, serial_send, register_number)
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
    serial: Path = typer.Argument(
        ...,
        help="Filename of the serial device",
        exists=True,
        writable=True,
        readable=True,
    ),
    register_number: int = typer.Argument(..., help="Register number", min=0),
    register_value: int = typer.Argument(
        ..., help="Register value to write to device", min=0
    ),
    timeout: float = typer.Option(
        DEFAULT_TIMEOUT, help="Timeout for register read", min=0.0
    ),
    serial_send: Optional[Path] = typer.Option(
        None,
        help="Filename of the serial device to write to. If present, SERIAL is only read from",
        exists=True,
        writable=True,
    ),
) -> None:
    if serial_send:
        typer.echo(
            f"Writing {register_value} (0x{register_value:02x}) to register {register_number} on send device {serial_send} and read device {serial}"
        )
    else:
        typer.echo(
            f"Writing {register_value} (0x{register_value:02x}) to register {register_number} on device {serial}"
        )
        serial_send = serial
    try:
        result = trio.run(
            run_write, timeout, serial, serial_send, register_number, register_value
        )
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
    serial: Path = typer.Argument(
        ...,
        help="Filename of the serial device",
        exists=True,
        writable=True,
        readable=True,
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
    # filename: Optional[Path] = typer.Option(
    #    None, help="Write received data to a file instead of STDOUT"
    # ),
    serial_send: Optional[Path] = typer.Option(
        None,
        help="Filename of the serial device to write to. If present, SERIAL is only read from",
        exists=True,
        writable=True,
    ),
) -> None:
    """
    With all the stop arguments and hitting Ctrl-C: whichever happens first will stop streaming.
    """
    if serial_send:
        typer.echo(
            f"Receive streaming data with send device {serial_send} and read device {serial}"
        )
    else:
        typer.echo(f"Receive streaming data with device {serial}")
        serial_send = serial
    try:
        trio.run(
            run_stream,
            stop_seconds,
            stop_messages,
            stop_bytes,
            stop_datasep,
            serial,
            serial_send,
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
