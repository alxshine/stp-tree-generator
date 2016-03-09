#pragma once

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <pcap.h>
#include <netinet/if_ether.h>

class Sniffer{
    private:
        const char * const filename = "log.txt";
        std::ofstream output;
        
    public:
        Sniffer();
        ~Sniffer();
        void start();
};
