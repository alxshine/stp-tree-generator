#include <iostream>

#include "../inc/Sniffer.hpp"

using namespace std;

int main(int argc, char **args){
    std::string filename;
    if(argc > 1){
        for(int c = 1; c<argc; c++){
            auto param = std::string(args[c]);
            if(param == "-f"){
                filename = std::string(args[c+1]);
            }
        }
    }
    try{
        Sniffer s = Sniffer::getInstance();
        s.start(filename);
    }catch (char const *c){
        cout << c << endl;
    }
}
