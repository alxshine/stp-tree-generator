#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <cctype>

#include "../inc/Server.hpp"

using namespace std;

int main(int argc, char **args){
    try{
        int port = 80;
        string outputFileName = "server.log";
        bool createPidFile = true;
        int timeout = 10;

        ifstream configRead("server.conf", ios_base::in);
        try{
            while(!configRead.eof()){
                string line;
                configRead >> line;
                if(line.length() == 0)
                    break;
                size_t colonLoc = line.find(":", 0);
                string parameter, value;
                parameter = line.substr(0, colonLoc);
                value = line.substr(colonLoc+1);
                if(parameter == "port")
                    port = stoi(value);
                else if(parameter == "outputFileName")
                        outputFileName = value;
                else if(parameter == "createPidFile"){
                    transform(value.begin(), value.end(), value.begin(), ::tolower);
                    createPidFile = value == "true";
                }
            }
        }catch(int a){
            cerr << "Error on reading config file";
        }

        for(int c = 1; c<argc; c++){
            auto param = string(args[c]);
            if(param == "-p")
                port = atoi(args[++c]);
            else if(param == "-of")
                outputFileName = string(args[++c]);
            else if(param == "-np")
                createPidFile = false;
            else if(param == "-t")
                timeout = atoi(args[++c]);
        }
 
        if(!configRead.is_open()){
            ofstream configWrite;
            configWrite.open("server.conf", ios_base::out);
            configWrite << "port:" << port << endl;
            configWrite << "outputFileName:" << outputFileName << endl;
            configWrite << "timeout:" << timeout << endl;

            configWrite.close();
        }        
        Server serv(port, outputFileName, createPidFile, timeout);
        
        serv.run();
    }catch(char const *c){
        cout << c << endl;
    }
}
