import socket

PORT = 9999
MESSAGE = "Start STP"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', 0))
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
print "broadcasting " + MESSAGE
sock.sendto(MESSAGE, ("<broadcast>", PORT))
