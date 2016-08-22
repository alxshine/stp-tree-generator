#include "../inc/Server.hpp"

Server::Server(int port, std::string outputFileName, bool createPidFile, time_t to){
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

    timeout = to;
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

        time_t currentTime = time(NULL);
        std::vector<int> toRemove;
        for(auto timeIt = timestamps.begin(); timeIt != timestamps.end(); timeIt++)
            if(timeIt->second < currentTime-timeout)
                toRemove.push_back(timeIt->first);

        for(int rId : toRemove){
            timestamps.erase(rId);
            clientData.erase(rId);
        }

        if(receivedJson["messagetype"] == "push"){
            SpanningTree newTree = SpanningTree::fromJson(receivedJson["tree"]);
            int id = receivedJson["id"].asInt();
      
            clientData[id] = newTree;
            timestamps[id] = time(NULL);

            output << "new data received, trees are now: " << std::endl;
            Json::FastWriter writer;
            for(auto mapIt = clientData.begin(); mapIt != clientData.end(); mapIt++)
                output << mapIt->first << ": " << writer.write(mapIt->second.toJson()) << std::endl;

            close(newsockfd);
        }else if(receivedJson["messagetype"] == "report"){
            std::vector<SpanningTree> trees;
            for(auto mapIt = clientData.begin(); mapIt != clientData.end(); mapIt++){
                bool contained = false;
                for(auto treeIt = trees.begin(); treeIt != trees.end(); treeIt++){
                    if(treeIt->containsRoot(mapIt->second) && *treeIt != mapIt->second){
                        *treeIt = *treeIt + mapIt->second;
                        contained = true;
                        break;
                    }
                }
                if(!contained)
                    trees.push_back(mapIt->second);
            }

            Json::Value jsonTrees;
            for(SpanningTree tree : trees)
                jsonTrees.append(tree.toJson());
            output << "trees requested" << std::endl;
            Json::FastWriter writer;
            std::string message = writer.write(jsonTrees);
            char buffer[message.length()+1];
            message.copy(buffer, message.length(), 0);
            buffer[message.length()] = 0;
            if(write(newsockfd, buffer, strlen(buffer))<0)
                throw "could not respond to client";
            close(newsockfd);
        }else if(receivedJson["messagetype"] == "register"){
            int id=0;
            std::cout << "register received\n";
            for(; clientData.find(id) == clientData.end(); id++)
                ;
            Json::Value retJson;
            retJson["id"] = id;
            
            output << "client registered" << std::endl;
            Json::FastWriter writer;
            std::string message = writer.write(retJson);
            char buffer[message.length()+1];
            message.copy(buffer, message.length(), 0);
            buffer[message.length()] = 0;
            if(write(newsockfd, buffer, strlen(buffer))<0)
                throw "could not respond to client";
            close(newsockfd);
        }
    }
}

