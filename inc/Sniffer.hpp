#pragma once

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

#include <pcap.h>
#include <netinet/if_ether.h>

#include "../inc/Bridge.hpp"
#include "../inc/Mac.hpp"
#include "../inc/SpanningTree.hpp"
#include "../inc/Client.hpp"
#include "../inc/json/json.h"
#include "../inc/json/json-forwards.h"

class Sniffer{
    private:
        static const char * const filename;
        static std::ofstream output;
        static void process_packet(u_char *user, const struct pcap_pkthdr *header, const u_char *bytes);
        static std::vector<Bridge> bridges;
        static Client * client;
        static SpanningTree getTree();
        static SpanningTree treeHelper(std::vector<Bridge>::iterator current, std::vector<Bridge>::iterator begin);

    public:
        Sniffer(bool noConnect, std::string outputFileName, std::string hostname, int port);
        void start(const std::string filename);
        ~Sniffer();
};
