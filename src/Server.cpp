#include "../inc/Server.hpp"

Server::Server(){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
        throw "error opening socket";

    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(31337);
    if(bind(sockfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0)
        throw "error on binding";

    if(listen(sockfd, 5)<0)
        throw "error on listen";

    clientLength = sizeof(clientAddress);

    output.open(filename, std::ios::app);
    if(!output.is_open())
        throw "could not open output file";

    currentTree = NULL;
}

Server::~Server(){
    close(sockfd);
    output.close();
    delete[] currentTree;
}

void Server::run(){
    while(1){
        newsockfd = accept(sockfd, (struct sockaddr*) &clientAddress, &clientLength);
        if(newsockfd <0)
            throw "error on accept";

        bzero(buffer, 256);
        int n = read(newsockfd,buffer,255);
        if(n<0)
            throw "error on read";
        
        auto receivedJson = nlohmann::json::parse(buffer);
        std::cout << receivedJson << std::endl;
        SpanningTree newTree = SpanningTree::fromJson(receivedJson);
        
        if(!currentTree)
            currentTree = new SpanningTree(newTree);
        else
            *currentTree = *currentTree + newTree;
        output << "new data received,tree is now: " << std::endl;
        output << currentTree->toJson() << std::endl;

        close(newsockfd);
    }
}

