#include <iostream>

#include "../inc/Sniffer.hpp"

using namespace std;


int main(int argc, char **args){
    string inputFileName, outputFileName = "client.log", hostname = "gl1tch.mooo.com";
    int port = 80;
    bool noConnect = false;

    ifstream configRead("client.conf", ios_base::in);
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
            transform(parameter.begin(), parameter.end(), parameter.begin(), ::tolower);

            if(parameter == "port")
                port = stoi(value);
            else if(parameter == "outputfilename")
                outputFileName = value;
            else if(parameter == "inputfilename")
                inputFileName = value;
            else if(parameter == "hostname")
                hostname = value;
            else if(parameter == "noconnect"){
                transform(value.begin(), value.end(), value.begin(), ::tolower);
                noConnect = value == "true";
            }
        }
    }catch(int i){
        cerr << "Error on reading config file";
    }

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
    if(!configRead.is_open()){
        ofstream configWrite;
        configWrite.open("client.conf", ios_base::out);
        configWrite << "hostname:" << hostname << endl;
        configWrite << "port:" << port << endl;
        configWrite << "outputFileName:" << outputFileName << endl;
        if(inputFileName.length() > 0)
            configWrite << "inputFileName:" << inputFileName << endl;
        configWrite << "noConnect:" << noConnect << endl;

        configWrite.close();
    }

    try{
        Sniffer s(noConnect, outputFileName, hostname, port);
        s.start(inputFileName);
    }catch (char const *c){
        cerr << c << endl;
    }
}
