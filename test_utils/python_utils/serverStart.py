import socket
import os

HOST = ''
PORT = 9999

servSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM);
print 'Socket created'

try:
    servSock.bind((HOST, PORT))
except socket.error, msg:
    print "Bind failed, error code: " + str(msg[0]) + ", message: " + msg[1]
    sys.exit()

print "socket bind complete"
while 1:
    data, addr = servSock.recvfrom(1024)
    print "received data: " + data

    os.system('/etc/set_stp')
