#include <iostream>

#include "../inc/Server.hpp"

using namespace std;

int main(int argc, char **args){
    try{
        int port = 80;
        string outputFileName = "server.log";
        bool createPidFile = true;
        
        for(int c = 1; c<argc; c++){
            auto param = string(args[c]);
            if(param == "-p")
                port = atoi(args[++c]);
            else if(param == "-of")
                outputFileName = string(args[++c]);
            else if(param == "-np")
                createPidFile = false;
        }
        Server serv(port, outputFileName, createPidFile);
        serv.run();
    }catch(char const *c){
        cout << c << endl;
    }
}
