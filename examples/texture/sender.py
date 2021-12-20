import SpoutGL
from math import ceil
import pygame
from OpenGL.GL import *
from OpenGL.GLU import *
from random import Random
import time
import argparse

TARGET_FPS = 30

DISPLAY_WIDTH = 800
DISPLAY_HEIGHT = 600
SEND_WIDTH = ceil(DISPLAY_WIDTH / 1)
SEND_HEIGHT = ceil(DISPLAY_HEIGHT / 1)

SENDER_NAME = "SpoutGL-texture-test"


VERTICES = (
    (1, -1, -1),
    (1, 1, -1),
    (-1, 1, -1),
    (-1, -1, -1),
    (1, -1, 1),
    (1, 1, 1),
    (-1, -1, 1),
    (-1, 1, 1)
)

EDGES = (
    (0, 1),
    (0, 3),
    (0, 4),
    (2, 1),
    (2, 3),
    (2, 7),
    (6, 3),
    (6, 4),
    (6, 7),
    (5, 1),
    (5, 4),
    (5, 7)
)


def Cube():
    random = Random(ceil(time.time()))

    glBegin(GL_LINES)
    for edge in EDGES:
        glColor3f(random.random(), random.random(), random.random())
        for index in edge:
            glVertex3fv(VERTICES[index])
    glEnd()


parser = argparse.ArgumentParser()
parser.add_argument('--use-send-fbo', dest='useSendFbo',
                    action='store_true')
args = parser.parse_args()

pygame.init()
pygame.display.set_caption('Texture Sender Example')
pygame.display.set_mode((DISPLAY_WIDTH, DISPLAY_HEIGHT),
                        pygame.OPENGL | pygame.DOUBLEBUF)

# Setup texture
sendTextureID = glGenTextures(1)
glBindTexture(GL_TEXTURE_2D, sendTextureID)
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0,
                 SEND_WIDTH, SEND_HEIGHT, 0)

# Create framebuffer and attach texture
fboID = glGenFramebuffers(1)
glBindFramebuffer(GL_FRAMEBUFFER, fboID)
glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sendTextureID, 0)

# Return to onscreen framebuffer
glBindFramebuffer(GL_FRAMEBUFFER, 0)

gluPerspective(45, (DISPLAY_WIDTH/DISPLAY_HEIGHT), 0.1, 50.0)
glTranslatef(0.0, 0.0, -5)

with SpoutGL.SpoutSender() as sender:
    sender.setSenderName(SENDER_NAME)

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                quit()

        # Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        # Rotate and draw cube
        glRotatef(1, 3, 3, 3)
        Cube()

        # Copy on-screen framebuffer to our fbo
        glBlitNamedFramebuffer(
            0, fboID, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, 0, SEND_WIDTH, SEND_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST)

        # Send the texture
        if args.useSendFbo:
            glBindFramebuffer(GL_FRAMEBUFFER, fboID)
            result = sender.sendFbo(fboID, SEND_WIDTH, SEND_HEIGHT, True)
            glBindFramebuffer(GL_FRAMEBUFFER, 0)
        else:
            result = sender.sendTexture(
                sendTextureID, GL_TEXTURE_2D, SEND_WIDTH, SEND_HEIGHT, True, 0)

        # Indicate that a frame is ready to read
        sender.setFrameSync(SENDER_NAME)

        # Update pygame display
        pygame.display.flip()

        # Wait for next frame
        pygame.time.wait(ceil(1000./TARGET_FPS))
