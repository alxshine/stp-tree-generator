import socket
import os

HOST = ''
PORT = 9999

servSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
print 'Socket created'

try:
    servSock.bind((HOST, PORT))
except socket.error, msg:
    print "Bind failed, error code: " + str(msg[0]) + ", message: " + msg[1]
    sys.exit()

print "socket bind complete"

servSock.listen(1)
print "socket listening"

connection, addr = servSock.accept()

os.system('mv /etc/config/network /etc/config/network_no_stp')
os.system('mv /etc/config/network_stp /etc/config/network')
os.system('/etc/init.d/network restart')
