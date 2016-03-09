#pragma once

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>

#include <pcap.h>
#include <netinet/if_ether.h>

class Sniffer{
    private:
        FILE *output;
        inline void ERRCHECK(int condition, const char *format, va_list arg);
        
    public:
        Sniffer(const char* filename);
        ~Sniffer();
        void start();
};
