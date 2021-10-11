import unittest
from threading import Thread
from itertools import islice, cycle, repeat
from random import randint
from io import BytesIO
import SpoutGL
from SpoutGL.helpers import isBufferEmpty, getBytesPerPixel
from SpoutGL.enums import GL_RGBA

def randcolor():
    return randint(0, 255)

FORMAT = GL_RGBA
SEND_WIDTH = 16
SEND_HEIGHT = 16

def run_as_thread(func):
    thread = Thread(target=func)
    thread.daemon = True
    thread.start()
    return thread

def make_single_color_image(width, height, format):
    """Make an image consisting of a single random color for all pixels"""
    return bytes(islice(cycle([randcolor(), randcolor(), randcolor(), 255]), SEND_WIDTH * SEND_HEIGHT * getBytesPerPixel(format)))

def make_empty_buffer(width, height, format):
    return BytesIO(bytes(repeat(0, width * height * getBytesPerPixel(format))))

class SendReceiveImageTest(unittest.TestCase):
    def test_send_receive_image(self):
        sender_name = "send_receive_image_test"
        done = False
        
        send_pixels = make_single_color_image(SEND_WIDTH, SEND_HEIGHT, FORMAT)
        received_pixels = None
    
        def run_sender():
            nonlocal done
        
            with SpoutGL.SpoutSender() as sender:
                sender.setSenderName(sender_name)
                
                while not done:
                   sender.sendImage(send_pixels, SEND_WIDTH, SEND_HEIGHT, FORMAT, False, 0)
            
        def run_receiver():
            nonlocal done, received_pixels
            buffer = None
        
            with SpoutGL.SpoutReceiver() as receiver:
                receiver.setReceiverName(sender_name)
                
                while not done:
                    if receiver.receiveImage(buffer.getbuffer() if buffer else None, FORMAT, False, 0):
                        if receiver.isUpdated():
                            incoming_width = receiver.getSenderWidth()
                            incoming_height = receiver.getSenderHeight()
                            buffer = make_empty_buffer(incoming_width, incoming_height, FORMAT)
                            continue
                    
                        received_pixels = buffer.getvalue()
                        
                        # Not sure why first frame is empty
                        if isBufferEmpty(buffer.getbuffer()):
                            continue
                        
                        done = True
    
        sender_thread = run_as_thread(run_sender)
        receiver_thread = run_as_thread(run_receiver)
        
        sender_thread.join()
        receiver_thread.join()

        assert send_pixels == received_pixels