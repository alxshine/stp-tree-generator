#include <cstring>
#include <iostream>
#include <fstream>
#include <map>

#include <stdio.h>
#include <unistd.h>
#include <time.h>
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
        time_t timeout;
        int id;

        std::ofstream output;
        std::map<int, SpanningTree> clientData;
        std::map<int, time_t> timestamps;

    public:
        Server(int port, std::string outputFileName, bool createPidFile, time_t to);
        ~Server();
        void run();
};
