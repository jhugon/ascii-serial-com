import trio
import trio.testing


class MemoryWriteStream:
    def __init__(self, stream):
        self.stream = stream

    async def write(self, data):
        await self.stream.send_all(data)

    async def flush(self):
        return


class MemoryReadStream:
    def __init__(self, stream):
        self.stream = stream

    async def read(self):
        return await self.stream.receive_some()


def breakStapledIntoWriteRead(stapledStream):
    return (
        MemoryWriteStream(stapledStream.send_stream),
        MemoryReadStream(stapledStream.receive_stream),
    )
