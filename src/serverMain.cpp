#include <iostream>

#include "../inc/Server.hpp"

using namespace std;

int main(void){
    try{
        Server serv;
        serv.run();
    }catch(char const *c){
        cout << c << endl;
    }
}
