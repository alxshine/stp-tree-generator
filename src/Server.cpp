#include "../inc/Server.hpp"

Server::Server(int port, std::string outputFileName, bool createPidFile){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
        throw "error opening socket";

    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
    if(bind(sockfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0)
        throw "error on binding";

    if(listen(sockfd, 5)<0)
        throw "error on listen";

    clientLength = sizeof(clientAddress);

    output.open(outputFileName, std::ios::app);
    if(!output.is_open())
        throw "could not open output file";

    output << "server established" << std::endl;

    //generate pid file
    if(createPidFile){
        pid_t id = getpid();
        std::ofstream pidOutput;
        pidOutput.open("server.pid");
        pidOutput << id << std::endl;
        pidOutput.close();
    }
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
        
        Json::Reader reader;
        Json::Value receivedJson;
        reader.parse(buffer, receivedJson);
        if(receivedJson["messagetype"] == "push"){
            SpanningTree newTree = SpanningTree::fromJson(receivedJson["tree"]);
      
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
            Json::FastWriter writer;
            for(SpanningTree tree : currentTrees)
                output << writer.write(tree.toJson()) << std::endl;

            close(newsockfd);
        }else if(receivedJson["messagetype"] == "report"){
            Json::Value trees;
            for(SpanningTree tree : currentTrees)
                trees.append(tree.toJson());
            output << "trees requested" << std::endl;
            Json::FastWriter writer;
            std::string message = writer.write(trees);
            char buffer[message.length()+1];
            message.copy(buffer, message.length(), 0);
            buffer[message.length()] = 0;
            if(write(newsockfd, buffer, strlen(buffer))<0)
                throw "could not respond to client";
        }
    }
}

