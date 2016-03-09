#include <iostream>

#include "../inc/Sniffer.hpp"

using namespace std;

int main(int argc, char **args){
    if(argc < 2){
        cout << "Usage: " << args[0] << " outputFilename";
        return -1;
    }
    Sniffer s(args[1]);
    s.start();
}
