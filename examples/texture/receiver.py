import SpoutGL
import pygame
from OpenGL.GL import *

DISPLAY_WIDTH = 800
DISPLAY_HEIGHT = 600

SENDER_NAME = "SpoutGL-texture-test"


def setProjection(width, height):
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glOrtho(0, width, height, 0, 1, -1)
    glMatrixMode(GL_MODELVIEW)


def drawSquare(width, height):
    glEnable(GL_TEXTURE_2D)
    glBegin(GL_QUADS)

    glTexCoord(0, 0)
    glVertex2f(0, 0)

    glTexCoord(1, 0)
    glVertex2f(width, 0)

    glTexCoord(1, 1)
    glVertex2f(width, height)

    glTexCoord(0, 1)
    glVertex2f(0, height)

    glEnd()
    glDisable(GL_TEXTURE_2D)


pygame.init()
pygame.display.set_caption('Texture Receiver Example')
pygame.display.set_mode((DISPLAY_WIDTH, DISPLAY_HEIGHT),
                        pygame.OPENGL | pygame.DOUBLEBUF)

receiveTextureID = glGenTextures(1)
glBindTexture(GL_TEXTURE_2D, receiveTextureID)
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0,
                 DISPLAY_WIDTH, DISPLAY_HEIGHT, 0)

setProjection(DISPLAY_WIDTH, DISPLAY_HEIGHT)
glClearColor(0.0, 0.0, 0.0, 1.0)

with SpoutGL.SpoutReceiver() as receiver:
    receiver.setReceiverName(SENDER_NAME)

    buffer = None
    width = 0
    height = 0

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                quit()

        result = receiver.receiveTexture(
            receiveTextureID, GL_TEXTURE_2D, False, 0)

        if receiver.isUpdated():
            width = receiver.getSenderWidth()
            height = receiver.getSenderHeight()

            print("Updated")

            # Initialize or update texture size
            glActiveTexture(GL_TEXTURE0)
            glBindTexture(GL_TEXTURE_2D, receiveTextureID)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,
                         height, 0, GL_RGBA, GL_UNSIGNED_BYTE, None)

            setProjection(width, height)

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        # Draw texture
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, receiveTextureID)
        drawSquare(width, height)

        pygame.display.flip()

        # Wait until the next frame is ready
        # Wait time is in milliseconds; note that 0 will return immediately
        receiver.waitFrameSync(SENDER_NAME, 1000)
