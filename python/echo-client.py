# echo-client.py

import sys
import trio


async def sender(outfilename):
    print(f"sender: started! with send file name: {outfilename}")
    async with await trio.open_file(outfilename, "wb", buffering=0) as f:
        for i in range(5):
            data = f"async is confusing {i}\n"
            data = data.encode()
            print(f"sender: sending {data!r}")
            await f.write(data)
            await f.flush()
            await trio.sleep(0.1)


async def receiver(infilename):
    print(f"receiver: started! with receive file name: {infilename}")
    async with await trio.open_file(infilename, "rb", buffering=0) as f:
        async for line in f:
            print(f"receiver received data: '{line!r}'", flush=True)
    print("receiver: file closed")
    sys.exit()


async def parent():
    print(f"parent: started!")
    async with trio.open_nursery() as nursery:
        print("parent: spawning sender...")
        nursery.start_soon(sender, sys.argv[2])

        print("parent: spawning receiver...")
        nursery.start_soon(receiver, sys.argv[1])


trio.run(parent)
