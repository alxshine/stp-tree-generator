#include "../inc/Client.hpp"

Client::Client(std::string hostname, int port){
    server = gethostbyname(hostname.c_str());
    if(!server)
        throw "could not resolve hostname";

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(port);
}

Client::~Client(){
}

void Client::send(std::string message){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        throw "could not create socket";

    char buffer[message.length()+1];
    message.copy(buffer, message.length(), 0);
    buffer[message.length()] = 0;

    if(connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
        throw "could not connect to server";

    if(write(sockfd, buffer, strlen(buffer))<0)
        throw "could not write to server";

    close(sockfd);
}
