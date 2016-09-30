#include "../inc/Server.hpp"

Server::Server(int port, std::string outputFileName, bool createPidFile, time_t to){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
        throw "error opening socket";
    id = 0;

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        throw "setsockopt(SO_REUSEADDR) failed";

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

std::vector<SpanningTree> Server::getTrees(){
    //remove invalid entries before creating SpanningTree
    //check every received vector against every other
    for(auto mapIt1 = clientData.begin(); mapIt1 != clientData.end(); ++mapIt1){
        std::vector<Bridge> vec1 = mapIt1->second;
        for(auto mapIt2 = clientData.begin(); mapIt2 != clientData.end(); ++mapIt2){
            if(mapIt1 == mapIt2)
                continue;
            std::vector<Bridge> vec2 = mapIt2->second;
            for(auto vecIt1 = vec1.begin(); vecIt1 != vec1.end(); ++vecIt1){
                for(auto vecIt2 = vec2.begin(); vecIt2 != vec2.end(); ++vecIt2){
                    //check every bridge against every other
                    //if one has a smaller message age, it is incorrect and needs to be removed
                    //this might overlook some errors if the number of clients is insufficient
                    if(vecIt1->getMessageAge() < vecIt2->getMessageAge()){
                        //remove bridge
                        vecIt1=vec1.erase(vecIt1);
                        //decrement vecIt1 to have it at the correct position in the next loop iteration
                        --vecIt1;
                        //continue with outer loop (vec1 iteration)
                        break;
                    }else if(vecIt2->getMessageAge() < vecIt1->getMessageAge()){
                        vecIt2=vec2.erase(vecIt2);
                        --vecIt2;
                        //no break needed -> next step is inner loop (vec2 iteration)
                    }
                }
            }
        }
    }

    //now that the incorrect entries are removed, we assume that everything else is correct
    //WHICH MAY STILL BE WRONG
    //create SpanningTrees like in Sniffer
    std::vector<SpanningTree> indivTrees = std::vector<SpanningTree>();
    for(auto mapIt = clientData.begin(); mapIt != clientData.end(); ++mapIt){
        auto vec = mapIt->second;
        std::sort(vec.begin(), vec.end(), [](Bridge a, Bridge b) {return a.getMessageAge() < b.getMessageAge();});
        if(vec.size() > 0)
            indivTrees.push_back(treeHelper(vec.begin(), vec.end()));
    }

    //now combine these trees
    std::vector<SpanningTree> ret;
    for(auto indivIt = indivTrees.begin(); indivIt != indivTrees.end(); ++indivIt){
        bool contained = false;
        for(auto treeIt = ret.begin(); treeIt != ret.end(); ++treeIt){
            if(treeIt->containsRoot(*indivIt) && *treeIt != *indivIt){
                *treeIt = *treeIt + *indivIt;
                contained = true;
                break;
            }
        }
        if(!contained)
            ret.push_back(*indivIt);
    }
    
    return ret;
}

SpanningTree Server::treeHelper(std::vector<Bridge>::iterator current, std::vector<Bridge>::iterator end){
    if(current >= end-1)
        return SpanningTree(*current);

    SpanningTree newTree(*current);
    newTree.addChild(treeHelper(++current, end));
    return newTree;
}


void Server::run(){
    while(1){
        newsockfd = accept(sockfd, (struct sockaddr*) &clientAddress, &clientLength);
        if(newsockfd <0)
            throw "error on accept";

        bzero(buffer, 1024);
        int n = read(newsockfd,buffer,1024);
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
            int id = receivedJson["id"].asInt();
            Json::Value jsonBridges = receivedJson["bridges"];
            clientData[id].clear();
            for(auto newbridge : jsonBridges){
                Bridge toAdd = Bridge::fromJson(newbridge);
                std::cout << toAdd.toTikz() << std::endl;
                clientData[id].push_back(toAdd);
            }
      
            timestamps[id] = time(NULL);

            Json::FastWriter writer;
            output << "new data received, data is now: " << std::endl;
            for(auto mapIt = clientData.begin(); mapIt != clientData.end(); mapIt++){
                output << mapIt->first << ": ";
                for(Bridge b : mapIt->second)
                    output << writer.write(b.toJson());
            }
            close(newsockfd);
        }else if(receivedJson["messagetype"] == "report"){
            std::vector<SpanningTree> trees = getTrees();

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
            clientData[id] = std::vector<Bridge>();
            timestamps[id] = time(NULL);

            Json::Value retJson;
            retJson["id"] = id++;
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

