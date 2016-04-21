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
}

Server::~Server(){
    close(sockfd);
    output.close();
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
  
        int contained = 0;
        for(auto current = currentTrees.begin(); current != currentTrees.end(); ++current){
            if(current->getRoot() == newTree.getRoot()){
                *current = *current + newTree;
                contained = 1;
                break;
            }
        }

        if(!contained)
            currentTrees.push_back(newTree);

        output << "new data received, trees are now: " << std::endl;
        for(SpanningTree tree : currentTrees)
            output << tree.toJson() << std::endl;

        close(newsockfd);
    }
}

