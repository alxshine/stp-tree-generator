#include <iostream>

#include "../inc/Server.hpp"

using namespace std;

int main(int argc, char **args){
    try{
        int port = 111;
        
        for(int c = 1; c<argc; c++){
            auto param = string(args[c]);
            if(param == "-p")
                port = atoi(args[++c]);
        }
        Server serv(port);
        serv.run();
    }catch(char const *c){
        cout << c << endl;
    }
}
