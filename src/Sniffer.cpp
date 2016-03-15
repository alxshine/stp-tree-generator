#include "../inc/Sniffer.hpp"

const int Sniffer::STP_PROTOCOL = 9728;
const char * const Sniffer::filename = "log.txt";
std::ofstream Sniffer::output;
Sniffer* Sniffer::reference;

Sniffer::Sniffer(){
    Sniffer::output.open(filename, std::ios::app);
    if(!Sniffer::output.is_open()){
        std::cout << "could not open file";
        exit(-1);
    }
}

Sniffer::~Sniffer(){
    Sniffer::output.flush();
    Sniffer::output.close();
}

void Sniffer::start(){
    Sniffer::output << "sniffer starting\n";
    Sniffer::output << "getting interfaces\n";
    pcap_if_t *alldevs;
    char err[100];
    if(pcap_findalldevs(&alldevs, err)){
        Sniffer::output << "error while getting interfaces, error code: " << err << std::endl;
        exit(-1);
    }

    pcap_if_t *device;
    Sniffer::output << "found devices:\n";
    for(device = alldevs; device != NULL; device = device->next){
        if(device->name){
            Sniffer::output << device->name;
            if(device->description)
                Sniffer::output << " - " << device->description;
            Sniffer::output << std::endl;
        }
    }

    Sniffer::output << "selecting first named device\n";
    for(device = alldevs; device != NULL && device->name == NULL; device = device->next)
        ;
    if(device == NULL){
        Sniffer::output << "no named device found\n";
        exit(-1);
    }

    Sniffer::output << "opening device " << device->name << " for sniffing" << std::endl;
    Sniffer::output.flush();
    pcap_t *capture_handle = pcap_open_live(device->name, 65536, 1, 0, err);
    if(!capture_handle){
        Sniffer::output << "could not start capture" << std::endl;
        exit(-1);
    }

    pcap_loop(capture_handle, -1, process_packet, NULL);
}

void Sniffer::process_packet(u_char *user, const struct pcap_pkthdr *header, const u_char *bytes){
    struct ethhdr *ethh = (struct ethhdr*) bytes;
    
    if(ethh->h_proto != Sniffer::STP_PROTOCOL)
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

    //one byte for version, bpdu type and flags respectively
    payload+=3;
    psize-=3;

    //root identifier
    //bridge priority is the next 2 bytes
    unsigned short rPriority;
    //actual priority is 4 bits
    rPriority = *((short*)payload);
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
    ////bridge priority is the next 2 bytes
    unsigned short bPriority;
    bPriority = *((short*)payload);
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
    short messageAge;
    messageAge = *((short*)payload);
    payload+=2;
    psize-=2;

    //generate Bridge objects
    Bridge root(rootMac, rPriority, 0);
    Bridge firsthop(bridgeMac, bPriority, messageAge);

    Sniffer::output << "Root: " << root << std::endl;
    Sniffer::output << "Next hop: " << firsthop << std::endl;
}

Sniffer& Sniffer::getInstance()
{
    if(Sniffer::reference==NULL)
        Sniffer::reference = new Sniffer();

    return *Sniffer::reference;
}

