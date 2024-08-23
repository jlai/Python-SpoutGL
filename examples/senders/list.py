import SpoutGL

with SpoutGL.SpoutReceiver() as receiver:
    active = receiver.getActiveSender()

    for sender_name in receiver.getSenderList():
        sender_info = receiver.getSenderInfo(sender_name)
        print("%s (%dx%d)%s" % (sender_name, sender_info.width, sender_info.height, " [active]" if sender_name == active else ""))
