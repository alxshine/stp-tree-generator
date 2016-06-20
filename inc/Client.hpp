#include <cstring>
#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "../inc/json/json.h"

class Client{
    private:
       int sockfd;
       int id;
       struct sockaddr_in serverAddress;
       struct hostent *server;

       void regServer();

    public:
       Client(std::string hostname, int port);
        ~Client();

        void send(Json::Value value);
};
