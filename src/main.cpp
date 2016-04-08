#include <iostream>

#include "../inc/Sniffer.hpp"

using namespace std;

int main(int argc, char **args){
    Sniffer& s = Sniffer::getInstance();
    s.start();
}
