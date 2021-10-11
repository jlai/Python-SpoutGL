import SpoutGL
from itertools import islice, cycle
import time
from OpenGL import GL
from random import randint

width = 2048
height = 2048


sender = SpoutGL.SpoutSender()
sender.setSenderName("test")

def randcolor():
    return randint(0, 255)

while True:
    pixels = bytes(islice(cycle([randcolor(), randcolor(), randcolor(), 255]), width * height * 4))

    result = sender.sendImage(pixels, width, height, GL.GL_RGBA, False, 0)
    print("Result", result)
    time.sleep(2)
