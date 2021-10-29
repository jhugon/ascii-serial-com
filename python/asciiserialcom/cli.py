from typing import Optional
from pathlib import Path
import logging

import typer
import trio

from asciiserialcom.asciiSerialCom import Ascii_Serial_Com

DEFAULT_TIMEOUT = 5

logging.basicConfig(
    # filename="test_asciiSerialCom.log",
    # level=logging.INFO,
    level=logging.DEBUG,
    format="%(levelname)s L%(lineno)d %(funcName)s: %(message)s",
)


async def run_read(timeout, fin_name, fout_name, reg_num):
    result = None
    with trio.move_on_after(timeout) as cancel_scope:
        async with await trio.open_file(fout_name, "bw") as fout:
            async with await trio.open_file(fin_name, "br") as fin:
                async with trio.open_nursery() as nursery:
                    asc = Ascii_Serial_Com(nursery, fin, fout, 8)
                    result = await asc.read_register(reg_num)
                    cancel_scope.cancel()
    return result


async def run_write(timeout, fin_name, fout_name, reg_num, reg_val):
    result = False
    with trio.move_on_after(timeout) as cancel_scope:
        async with await trio.open_file(fout_name, "bw") as fout:
            async with await trio.open_file(fin_name, "br") as fin:
                async with trio.open_nursery() as nursery:
                    asc = Ascii_Serial_Com(nursery, fin, fout, 8)
                    await asc.write_register(reg_num, reg_val)
                    result = True
                    cancel_scope.cancel()
    return result


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
    timeout: float = typer.Argument(
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
    filename: Optional[Path] = typer.Option(
        None, help="Write received data to a file instead of STDOUT"
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
    if filename:
        typer.echo(f"Writing to path: {filename}")


def main():
    app()
