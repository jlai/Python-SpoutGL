import SpoutGL
from OpenGL import GL
from itertools import repeat
import array

SENDER_NAME = "SpoutGL-test"

with SpoutGL.SpoutReceiver() as receiver:
    receiver.setReceiverName(SENDER_NAME)

    buffer = None
    shared_memory = None

    while True:
        result = receiver.receiveImage(buffer, GL.GL_RGBA, False, 0)

        if receiver.isUpdated():
            width = receiver.getSenderWidth()
            height = receiver.getSenderHeight()
            buffer = array.array('B', repeat(0, width * height * 4))

        if buffer and result and not SpoutGL.helpers.isBufferEmpty(buffer):
            print("Got bytes", bytes(buffer[0:64]), "...")

        # Wait until the next frame is ready
        # Wait time is in milliseconds; note that 0 will return immediately
        receiver.waitFrameSync(SENDER_NAME, 10000)
        print("sync received")

        if shared_memory is None:
            memory_length = receiver.getMemoryBufferSize(SENDER_NAME)
            shared_memory = array.array('B', repeat(0, memory_length))

        result = receiver.readMemoryBuffer(SENDER_NAME, shared_memory, len(shared_memory))
        if result:
            message = bytes(shared_memory).decode().split("\x00")[0]
            print(message)
