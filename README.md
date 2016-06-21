# stp-tree-generator
Generates STP trees based on multiple nodes observing stp packets.
The nodes do not have to be very powerful, as the combination of data is done on the server; and even there it is not particularly intensive.

## Installation Requirements
In order to compile the client side the pcap library is needed.
The server side can be compiled using only standard libraries.
For the client-server communication i used JSON (in part to keep it independent from C) and the jsoncpp library.
The version used should be around 1.7.2 and requires C++11 to compile.

## Building
After pulling the git, simply run make to build both the client and server.
You can also build only the client, server and parser using make and the desired component.
After building is complete, the binaries will be in the bin folder.

## Hardware Configuration
The sniffer uses the first named interface.
Usually deactivating  WI-FI (if it is available) is enough to ensure the correct interface is used.

##Command Line Parameters
The server, client and parser can be called via the following command line parameters respectively.
###Server
####-np
Do **not** create a PID file. Will create one by default.
####-of
Set the file name of the output file **Default: server.log**
####-p
Set the port the client will try to connect to. **Default: 80**
####-t
Set the timeout for the clients in seconds. **Default: 5s**

###Client
The client needs to be run as root in order to capture on an interface.
####-h
Set the host the client will connect to (as URL).
####-if
If set, this will specify a __.pcapng__ file to read from. Note that this will disable live capturing on an interface.
####-of
Set the name for the outputfile. **Default: client.log**
####-p
Set the port the client will try to connect to. **Default: 80**
