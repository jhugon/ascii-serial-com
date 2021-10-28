import trio
import trio.testing


class MemoryWriteStream:
    """
    Makes a network socket behave like a file
    """

    def __init__(self, stream):
        self.stream = stream

    async def write(self, data):
        await self.stream.send_all(data)

    async def flush(self):
        return


class MemoryReadStream:
    """
    Makes a network socket behave like a file
    """

    def __init__(self, stream):
        self.stream = stream

    async def read(self):
        return await self.stream.receive_some()


def breakStapledIntoWriteRead(stapledStream):
    return (
        MemoryWriteStream(stapledStream.send_stream),
        MemoryReadStream(stapledStream.receive_stream),
    )


class ChannelWriteStream:
    """
    Makes a channel behave like a file
    """

    def __init__(self, chan):
        self.chan = chan

    async def write(self, data):
        await self.chan.send(data)

    async def flush(self):
        return


class ChannelReadStream:
    """
    Makes a channel behave like a file
    """

    def __init__(self, chan):
        self.chan = chan

    async def read(self):
        return await self.chan.receive()
