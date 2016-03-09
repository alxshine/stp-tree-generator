#include "../inc/Sniffer.hpp"

Sniffer::Sniffer(){
    output.open(filename, std::ios::app);
    if(!output.is_open()){
        std::cout << "could not open file";
        exit(-1);
    }
}

Sniffer::~Sniffer(){
    output.flush();
    output.close();
}

void Sniffer::start(){
    output << "sniffer starting\n";
    output << "getting interfaces\n";
    pcap_if_t *alldevs;
    char err[100];
    if(pcap_findalldevs(&alldevs, err)){
        output << "error while getting interfaces, error code: " << err << std::endl;
        exit(-1);
    }

    pcap_if_t *device;
    output << "found devices:\n";
    for(device = alldevs; device != NULL; device = device->next)
        output << device->name << "-" <<  device->description;

    output << "selecting first named device\n";
    for(device = alldevs; device != NULL && device->name == NULL; device = device->next)
        ;
    if(device == NULL){
        output << "no named device found\n";
        exit(-1);
    }
}

