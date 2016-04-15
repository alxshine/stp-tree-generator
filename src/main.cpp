#include <iostream>
#include <thread>

#include "../inc/Sniffer.hpp"

using namespace std;

int main(int argc, char **args){
    Sniffer s = Sniffer::getInstance();
    s.start();
    this_thread::sleep_for(chrono::seconds(3));
    cout << s.getTree() << endl;
}
