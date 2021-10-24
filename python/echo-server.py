# echo-server.py

import sys
import trio


async def echo_server(infilename, outfilename):
    print(
        f"echo_server: started with infilename: {infilename} and outfilename: {outfilename}"
    )
    try:
        async with await trio.open_file(infilename, "rb", buffering=0) as infile:
            async with await trio.open_file(outfilename, "wb", buffering=0) as outfile:
                async for line in infile:
                    print(f"echo_server received data: '{line!r}'", flush=True)
                    await outfile.write(line)
        print(f"echo_server: files closed")
    # FIXME: add discussion of MultiErrors to the tutorial, and use
    # MultiError.catch here. (Not important in this case, but important if the
    # server code uses nurseries internally.)
    except Exception as exc:
        # Unhandled exceptions will propagate into our parent and take
        # down the whole program. If the exception is KeyboardInterrupt,
        # that's what we want, but otherwise maybe not...
        print(f"echo_server: crashed: {exc!r}")


async def main():
    infilename = sys.argv[1]
    outfilename = sys.argv[2]
    print(f"main started with infilename: {infilename} and outfilename: {outfilename}")
    await echo_server(infilename, outfilename)


trio.run(main)
