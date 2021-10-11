import SpoutGL
from itertools import islice, cycle
import time
from OpenGL import GL
from random import randint

TARGET_FPS = 1
SEND_WIDTH = 256
SEND_HEIGHT = 256
SENDER_NAME = "SpoutGL-test"

def randcolor():
    return randint(0, 255)

with SpoutGL.SpoutSender() as sender:
    sender.setSenderName(SENDER_NAME)

    while True:
        # Generating bytes in Python is very slow; ideally you should pass in a buffer obtained elsewhere
        # or re-use an already allocated array instead of allocating one on the fly
        pixels = bytes(islice(cycle([randcolor(), randcolor(), randcolor(), 255]), SEND_WIDTH * SEND_HEIGHT * 4))

        result = sender.sendImage(pixels, SEND_WIDTH, SEND_HEIGHT, GL.GL_RGBA, False, 0)
        print("Send result", result)
        
        # Indicate that a frame is ready to read
        sender.setFrameSync(SENDER_NAME)
        
        # Wait for next send attempt
        time.sleep(1./TARGET_FPS)
