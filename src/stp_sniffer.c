#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/if_ether.h>

#include <pcap.h>

#define LOGFILE "log.txt"
#define NAMESIZE 256
#define STP_PROTOCOL 9728

#define ERRCHECK(__cond, __message, ...) \
    if(__cond){ \
        fprintf(output, "Error in %s#%d\n", __FILE__, __LINE__); \
        fprintf(output, __message "\n", ##__VA_ARGS__); \
        exit(-1); \
    }

#define LOG(__message, ...)\
    fprintf(output, __message "\n", ##__VA_ARGS__);\
    fflush(output);

FILE *output;

void printMac(char *string, const u_char *bytes){
//    sprintf(string, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]);
    for(int i=0; i<5; i++)
        sprintf(string+i*3, "%.2X:", bytes[i]);
    sprintf(string+15, "%.2X", bytes[5]);
}

//-----------------------------------------------------
void process_packet(u_char *user, const struct pcap_pkthdr *header, const u_char *bytes){
    struct ethhdr *ethh = (struct ethhdr*) bytes;
    
    if(ethh->h_proto != STP_PROTOCOL)
        return;

    //print ethernet header
    LOG("\nETHERNET HEADER");
    LOG("\t|-Destination Address:\t%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", ethh->h_dest[0], ethh->h_dest[1], ethh->h_dest[2], ethh->h_dest[3], ethh->h_dest[4], ethh->h_dest[5]);
    LOG("Destination String: %c%c%c%c%c%c", ethh->h_dest[0], ethh->h_dest[1], ethh->h_dest[2], ethh->h_dest[3], ethh->h_dest[4], ethh->h_dest[5]);
    LOG("\t|-Source Address:\t%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", ethh->h_source[0], ethh->h_source[1], ethh->h_source[2], ethh->h_source[3], ethh->h_source[4], ethh->h_source[5]);
    LOG("\t|-Protocol:\t\t%u", ethh->h_proto);

    //print payload
    LOG("PAYLOAD");
    int size = header->len;
    LOG("size: %d", size);
    int stringlength = 3*size + size/32 + 1;
    char outstring[stringlength];
    for(int i=0, j=0; i<size; i++, j+=3){
        if(i%16==0 && i>0){
            sprintf(outstring+j, "\n");
            j++;
        }
        sprintf(outstring+j, "%02X ", bytes[i]);
    }
    LOG("%s", outstring);

    //extract bridge id, root id, priority and distance
    LOG("\nSTP VALUES");
    
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
    LOG("ROOT:");
    //bridge priority is the next 2 bytes
    unsigned short rPriority, rIdExtension;
    //actual priority is 4 bits
    rPriority = *payload>>4;
    LOG("\tpriority: %d", rPriority);
    //next 12 is extended id
    //add first 4 bits shifted
    rIdExtension = (*payload++ & 0b1111) << 8;
    //add rest
    rIdExtension += *(payload++);
    psize-=2;
    LOG("\tidExtension: %X", rIdExtension);

    //next 6 bytes is the root bridge id
    char rootId[17];
    printMac(rootId, payload);
    payload+=6;
    psize-=6;
    LOG("\troot mac: %s", rootId);
    LOG("\troot id: %d / %X / %s", rPriority, rIdExtension, rootId);

    //root path cost (4 bytes)
    int rootPathCost;
    memcpy(&rootPathCost, payload, 4);
    payload+=4;
    psize-=4;
    LOG("\troot path cost: %d", rootPathCost);

    //bridge identifier
    LOG("BRIDGE:");
    ////bridge priority is the next 2 bytes
    unsigned short sPriority, sIdExtension;
    //actual priority is 4 bits
    sPriority = *payload>>4;
    LOG("\tpriority: %d", sPriority);
    //next 12 is extended id
    //add first 4 bits shifted
    sIdExtension = (*payload++ & 0b1111) << 8;
    //add rest
    sIdExtension += *(payload++);
    psize-=2;
    LOG("\tidExtension: %X", sIdExtension);

    //next 6 bytes is the bridge mac address
    char bridgeId[17];
    printMac(bridgeId, payload);
    payload+=6;
    psize-=6;
    LOG("\tbridge mac: %s", bridgeId);
    LOG("\tbridge id: %d / %X / %s", sPriority, sIdExtension, bridgeId);

    //next 2 bytes are port identifier
    payload+=2;
    psize-=2;

    //next 2 bytes is message age
    short messageAge;
    memcpy(&messageAge, payload, 2);
    payload+=2;
    psize-=2;
    LOG("message age: %d", messageAge);
}

int main(int argc, char **args){
    char filename[NAMESIZE];
    if(argc >= 2)
        strncpy(filename, args[1], NAMESIZE);
    else
        sprintf(filename, "%s", LOGFILE);

    if(argc >= 2 && !strcmp(args[1], "stdout"))
        output = stdout;
    else
        output = fopen(filename, "a");

    ERRCHECK(output == NULL, "Error opening file %s", filename);

    LOG("\ngetting interfaces");
    pcap_if_t *alldevs, *device;
    char err[100];
    ERRCHECK(pcap_findalldevs(&alldevs, err), "Error finding devices: %s", err);

    LOG("\nFound devices:");
    for(device = alldevs; device != NULL; device = device->next)
        LOG("%s - %s", device->name, device->description);

    LOG("\nfinding first device");
    for(device = alldevs; device != NULL && device->name == NULL; device = device->next) ;
    ERRCHECK(device->name == NULL, "Could not find a named device");

    LOG("\nOpening device %s for sniffing", device->name);
    pcap_t *capture_handle = pcap_open_live(device->name, 65536, 1, 0, err);
    ERRCHECK(!capture_handle, "Could not open device %s, error: %s", device->name, err);

    pcap_loop(capture_handle, -1, process_packet, NULL);

    return 0;
}
