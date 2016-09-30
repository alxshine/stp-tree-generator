#include "../inc/Sniffer.hpp"

std::ofstream Sniffer::output;
std::vector<Bridge> Sniffer::bridges;
Client * Sniffer::client;
bool Sniffer::hadTC;
bool Sniffer::noConnect;

Sniffer::Sniffer(bool nC, std::string outputFileName, std::string hostname, int port){
    bridges = std::vector<Bridge>();
    Sniffer::output.open(outputFileName, std::ios::app);
    if(!Sniffer::output.is_open()){
        std::cerr << "could not open file" << std::endl;
        exit(-1);
    }

    noConnect = nC;
    if(noConnect){
        client = NULL;
    }else{
        client = new Client(hostname, port);
    }
    hadTC = false;
}

Sniffer::~Sniffer(){
    output.flush();
    output.close();
    delete client;
}

void Sniffer::start(const std::string inputFileName, const std::string deviceName){
    output << "sniffer starting\n";
    if(!noConnect){
        output << "registering client\n";
        client->regServer();
    }
    char err[PCAP_ERRBUF_SIZE];
    if(inputFileName.size() == 0){
        output << "getting interfaces\n";
        pcap_if_t *alldevs;
        if(pcap_findalldevs(&alldevs, err)){
            output << "error while getting interfaces, error code: " << err << std::endl;
            exit(-1);
        }

        pcap_if_t *device;
        output << "found devices:\n";
        for(device = alldevs; device != NULL; device = device->next){
            if(device->name){
                output << device->name;
                if(device->description)
                    output << " - " << device->description;
                output << std::endl;
            }
        }

        if(deviceName == ""){
            output << "selecting first named device\n";
            for(device = alldevs; device != NULL && device->name == NULL; device = device->next)
                ;
            if(device == NULL){
                output << "no named device found\n";
                exit(-1);
            }
        }else{
            for(device = alldevs; device != NULL && device->name != deviceName; device = device->next)
                ;
            if(device == NULL){
                output << "desired device not found\n";
                exit(-1);
            }
        }

        //save device name for restart
        std::string devname = std::string(device->name);

        output << "opening device " << device->name << " for sniffing" << std::endl;
        pcap_t *capture_handle = pcap_open_live(device->name, 65536, 1, 0, err);
        if(!capture_handle){
            output << "could not start capture" << std::endl;
            exit(-1);
        }

        while(1){
            pcap_loop(capture_handle, -1, process_packet, NULL);
            output << "interface disconnected, closing capture\n";
            pcap_close(capture_handle);
            capture_handle = NULL;

            output << "trying to find device " << devname << " again\n";
            while(capture_handle == NULL){
                sleep(2);
                pcap_if_t *alldevs;
                if(pcap_findalldevs(&alldevs, err)){
                    output << "error while getting interfaces, error code: " << err << std::endl;
                    exit(-1);
                }

                for(device = alldevs; device != NULL && device->name != devname; device = device->next)
                    ;
                if(device != NULL){
                    capture_handle = pcap_open_live(device->name, 65536, 1, 0, err);
                    if(!capture_handle){
                        output << "could not start capture" << std::endl;
                        exit(-1);
                    }
                }
            }
            output << "device found, restarting capture\n";
        }
    }else{
        output << "reading from file\n";
        pcap_t *pcap = pcap_open_offline(inputFileName.c_str(), err);
        struct pcap_pkthdr *header;
        const u_char *data;
        while(pcap_next_ex(pcap, &header, &data) >= 0)
            process_packet(NULL, header, data);
    }
}

void Sniffer::process_packet(u_char *user, const struct pcap_pkthdr *header, const u_char *bytes){
    struct ethhdr *ethh = (struct ethhdr*) bytes;

    char buffer[17];
    sprintf(buffer,"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", ethh->h_dest[0], ethh->h_dest[1], ethh->h_dest[2], ethh->h_dest[3], ethh->h_dest[4], ethh->h_dest[5]);
    if(std::string(buffer, 17) != "01:80:C2:00:00:00")
        return;

    //handle payload
    const u_char *payload = bytes+sizeof(struct ethhdr);
    int psize = header->len - sizeof(struct ethhdr);
    //payload starts at logical link control level
    //llc has 3 bytes
    payload+=3;
    psize-=3;
    //payload is now at the start of the stp payload
    //protocol identifier stp (2 bytes)
    payload+=2;
    psize-=2;

    //one byte for version and bpdu type
    payload+=2;
    psize-=2;

    //tc flag
    bool tc = *(payload++);
    psize--;

    //root identifier
    //bridge priority is the next 2 bytes
    //actual bridge priority is 4 bits, 
    //then comes id extension with 12 bits
    
    //generate filters
    unsigned short extFilter = -1;
    extFilter = extFilter>>4;
    unsigned short prioFilter = extFilter<<12;
    
    unsigned short rPriority, rExtension;
    rPriority = htons(*((short*)payload));
    rExtension = rPriority & extFilter;
    rPriority = rPriority & prioFilter;
    payload+=2;
    psize-=2;
    //next 6 bytes is the root bridge id
    Mac rootMac(payload);
    payload+=6;
    psize-=6;

    //root path cost (4 bytes)
    payload+=4;
    psize-=4;

    //bridge identifier
    //bridge priority is the next 2 bytes
    unsigned short bPriority, bExtension;
    bPriority = htons(*((short*)payload));
    bExtension = bPriority & extFilter;
    bPriority = bPriority & prioFilter;
    payload+=2;
    psize-=2;
    //next 6 bytes is the bridge mac address
    Mac bridgeMac(payload);
    payload+=6;
    psize-=6;

    //next 2 bytes are port identifier
    payload+=2;
    psize-=2;

    //next 2 bytes is message age
    unsigned short messageAge;
    messageAge = *((short*)payload);
    payload+=2;
    psize-=2;

    //generate Bridge objects
    Bridge root(rootMac, rPriority, rExtension, 0);
    Bridge firstHop(bridgeMac, bPriority, bExtension, messageAge);

    //check if the two nodes are contained
    int rootContained = 0, oldHopMa = -1;
    for(Bridge b : bridges){
        if(b == root)
            rootContained = 1;
        if(b == firstHop)
            oldHopMa = b.getMessageAge();
    }

    //handle network changes
    if(rootContained){
        if(oldHopMa >= 0){
            //both contained
            if(oldHopMa != firstHop.getMessageAge()){
                //upstream changes
                clearAndAdd(firstHop, root);
            }else{
                //everything is as it was
                //if something changed upstream a tc flag will be sent
                //->reset
                if(tc && !hadTC)
                    clearAndAdd(firstHop, root);
            }
        }else{
            //first hop not contained
            //this means that the node was plugged in on a different bridge
            //(most likely)
            //we don't know how long we were disconnected, so reregister the client
            if(!noConnect)
                client->regServer();
            clearAndAdd(firstHop, root);
        }
    }else{
        //root not contained
        if(oldHopMa >= 0){
            //if the first hop was contained this means there were some changes upstream
            //the root moving away means we assume network growth
            if(oldHopMa < firstHop.getMessageAge()){
                std::cout << "root moved away\n";
                int maDiff = firstHop.getMessageAge() - oldHopMa;
                //this means we have to increase every message age and add the new root
                for(Bridge &b : bridges)
                    b.setMessageAge(b.getMessageAge() + maDiff);
                bridges.push_back(root);
            }else{
                clearAndAdd(firstHop, root);
            }
        }else{
            //entirely new setup
            clearAndAdd(firstHop, root);
        }
    }
    hadTC = tc;

    Json::FastWriter writer;
    Json::Value toSend;
    toSend["messagetype"] = "push";
    Json::Value jsonBridges = Json::arrayValue;
    for(Bridge b : bridges)
        jsonBridges.append(b.toJson());
    toSend["bridges"] = jsonBridges;
    output << "sniffer is sending: " << std::endl << writer.write(toSend) << std::endl;
    try{
        if(client)
            client->send(toSend);
    }catch(const char * msg){
        output << msg << std::endl;
    }
}

void Sniffer::clearAndAdd(Bridge firstHop, Bridge root){
    bridges.clear();
    if(firstHop != root)
        bridges.push_back(root);
    bridges.push_back(firstHop);
}
