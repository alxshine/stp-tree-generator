#include <iostream>

#include "../inc/Sniffer.hpp"

using namespace std;


int main(int argc, char **args){
    string inputFileName, outputFileName = "client.log", hostname = "gl1tch.mooo.com";
    int port = 80;
    bool noConnect = false;

    for(int c = 1; c<argc; c++){
        auto param = string(args[c]);
        if(param == "-if")
            inputFileName = std::string(args[++c]);
        else if(param == "--no-connect" || param == "-n")
            noConnect = true;
        else if(param == "-of")
            outputFileName = std::string(args[++c]);
        else if(param == "-p")
            port = atoi(args[++c]);
        else if(param == "-h")
            hostname = args[++c];
        
    }
    try{
        Sniffer s(noConnect, outputFileName, hostname, port);
        s.start(inputFileName);
    }catch (char const *c){
        cout << c << endl;
    }
}
