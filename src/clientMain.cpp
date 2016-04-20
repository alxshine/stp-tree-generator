#include <iostream>

#include "../inc/Sniffer.hpp"

using namespace std;

int main(void){
    try{
        Sniffer s = Sniffer::getInstance();
        s.start();
    }catch (char const *c){
        cout << c << endl;
    }
}
