#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../inc/SpanningTree.hpp"
#include "../inc/json/json.h"
#include "../inc/json/json-forwards.h"

class Server{
    private:
        int sockfd, newsockfd;
        socklen_t clientLength;
        char buffer[256];
        struct sockaddr_in serverAddress, clientAddress;

        std::ofstream output;
        const char * const filename = "server.log";
        std::vector<SpanningTree> currentTrees;

    public:
        Server();
        ~Server();
        void run();
};
