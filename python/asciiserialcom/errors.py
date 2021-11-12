"""
Errors and Exceptions for ASCII Serial Com Python Interface
"""

import typing


def printError(error: Exception) -> None:
    args = error.args
    argsStr = " ".join([str(x) for x in args])
    print(f"{type(error).__name__}: {argsStr}")


class ASCErrorBase(Exception):
    """
    Abstract base class for Ascii-Serial-Com Errors
    """


class MalformedFrameError(ASCErrorBase):
    """
    Message frame does not meet specification
    """


class ResponseTimeoutError(ASCErrorBase):
    """
    Timeout while waiting for response frame
    """


class VersionMismatchErrorBase(ASCErrorBase):
    """
    Abstract base class for version mismatch errors
    """


class AsciiSerialComVersionMismatchError(VersionMismatchErrorBase):
    """
    Different ASC version in a received frame than expected.
    """


class ApplicationVersionMismatchError(VersionMismatchErrorBase):
    """
    Different application version byte received in a frame than expected.
    """


class BadCommandError(ASCErrorBase):
    """
    Frame command byte is not valid
    """


class BadDataError(ASCErrorBase):
    """
    Frame data section is not valid
    """


class BadRegisterNumberError(ASCErrorBase):
    """
    Register number is not valid
    """


class BadRegisterContentError(ASCErrorBase):
    """
    Register content is not valid
    """


class BadStreamMsgNumberError(ASCErrorBase):
    """
    Stream message number is not valid
    """


class TextFileNotAllowedError(ASCErrorBase):
    """
    Stream files must be opened in binary mode
    """


class ConfigurationError(ASCErrorBase):
    """
    __init__ args don't make sense
    """


class MessageIntegrityError(ASCErrorBase):
    """
    Corrupted message data
    """


class ShellArgumentError(ASCErrorBase):
    """
    Incorrect argument(s) to shell command
    """


class FileReadError(ASCErrorBase):
    """
    Incorrect argument(s) to shell command
    """
