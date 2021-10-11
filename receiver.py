import SpoutGL
import time
from OpenGL import GL
from io import BytesIO
from itertools import repeat
from zlib import adler32

with SpoutGL.SpoutReceiver() as r:
    r.setReceiverName("test")

    buffer = BytesIO(bytes(repeat(0, 1024 * 1024 * 4)))

    while True:
        print("Frame", r.getSenderFrame())
    
        if r.isUpdated():
            width = r.getSenderWidth()
            height = r.getSenderHeight()
            buffer = BytesIO(bytes(repeat(0, width * height * 4)))

        result = r.receiveImage(buffer.getbuffer(), GL.GL_RGBA, False, 0)
        if result:
            print("Got bytes", adler32(buffer.getbuffer()), bytes(buffer.getbuffer()[0:4]))
        time.sleep(1)
        
    print("Releasing")

print("Done")