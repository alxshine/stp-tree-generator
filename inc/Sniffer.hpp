#pragma once

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include <pcap.h>
#include <netinet/if_ether.h>

#include "Bridge.hpp"
#include "Mac.hpp"
#include "SpanningTree.hpp"

class Sniffer{
    private:
        static const char * const filename;
        static const int STP_PROTOCOL;
        static std::ofstream output;
        static void process_packet(u_char *user, const struct pcap_pkthdr *header, const u_char *bytes);
        static Sniffer * reference;
        static std::vector<Bridge> bridges;
        Sniffer();

    public:
        static Sniffer& getInstance();
        void start();
        SpanningTree getTree();
        ~Sniffer();
};
