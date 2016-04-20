#include <cstring>
#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

class Client{
    private:
       int sockfd;
       struct sockaddr_in serverAddress;
       struct hostent *server;

    public:
       Client();
        ~Client();

        void send(std::string);
};
