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

void Client::regServer(){
    Json::Value regMessage;
    regMessage["messagetype"] = "register";
    Json::FastWriter writer;
    std::string message = writer.write(regMessage);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        throw "could not create socket";

    char buffer[256];
    message.copy(buffer, message.length(), 0);
    buffer[message.length()] = 0;

    if(connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
        throw "could not connect to server";

    if(write(sockfd, buffer, strlen(buffer))<0)
        throw "could not write to server";

    //reset buffer
    for(int i=0; i<256; i++)
        buffer[i] = 0;
    
    if(read(sockfd, buffer, 255) < 0)
        throw "error on read";

    Json::Reader reader;
    Json::Value recVal;
    reader.parse(buffer, recVal);
    id = recVal["id"].asInt();
    std::cout << "client id is: " << id << std::endl;
    close(sockfd);
}

Client::~Client(){
}

void Client::send(Json::Value value){
    value["id"] = id;
    Json::FastWriter writer;
    std::string message = writer.write(value);

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

    std::cout << time(0) << " sending: " << buffer << std::endl;
    close(sockfd);
}
