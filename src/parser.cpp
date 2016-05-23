#include <iostream>
#include <fstream>

#include<unistd.h>
#include<sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "../inc/SpanningTree.hpp"

using namespace std;

int main(int argc, char ** args){
    string hostname = "gl1tch.mooo.com";
    int port = 80;
    int buffersize = 4096;

    for(int c = 1; c<argc; c++){
        auto param = string(args[c]);
        if(param == "-p")
            port = atoi(args[++c]);
        else if(param == "-h")
            hostname = args[++c];
        else if(param == "-bs")
            buffersize = atoi(args[++c]);
    }
    
    try{
        struct hostent *server = gethostbyname(hostname.c_str());
        if(!server)
            throw "could not resolve hostname";

        struct sockaddr_in serverAddress;
        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
        serverAddress.sin_port = htons(port);
        
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd<0)
            throw "could not create socket";

        if(connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
            throw "could not connect to server";

        Json::Value toSend;
        toSend["messagetype"] = "report";
        Json::FastWriter writer;
        string sendMessage = writer.write(toSend);
        char sendBuffer[sendMessage.length()+1];
        sendMessage.copy(sendBuffer, sendMessage.length(), 0);
        sendBuffer[sendMessage.length()] = 0;

        if(write(sockfd, sendBuffer, strlen(sendBuffer))<0)
            throw "could not write to server";

        char receiveBuffer[buffersize];
        if(read(sockfd, receiveBuffer, buffersize) < 0)
            throw "could not read from server";

        Json::Reader reader;
        Json::Value jsonValue;
        reader.parse(receiveBuffer, jsonValue);
        
        for(Json::Value treejson : jsonValue){
            SpanningTree currentTree = SpanningTree::fromJson(treejson);

            string filename = currentTree.getRoot().getMac().getAddress();
            ofstream outputFile(filename + ".tikz", std::ios::out);
            if(!outputFile.is_open()){
                cout << "could not open " + filename + ".tikz";
                return -1;
            }

            outputFile << "\\begin{tikzpicture}[transform shape]" << endl;
            outputFile << currentTree.toTikz(0, 100, 0, 20, 0) << endl;

            outputFile << "\\end{tikzpicture}";
            outputFile.close();
        }
    }catch(string s){
        cout << s << std::endl;
    }
}
