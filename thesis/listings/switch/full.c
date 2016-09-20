#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <pthread.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <pcap/pcap.h>
#include <netinet/if_ether.h>

//the raw socket creation and stp packet sending is loosely based on the code of The Linux Channel
//https://www.youtube.com/watch?v=UyQyPB1GX3U

typedef enum port_state {
    ROOT,
    DEDICATED,
    BLOCKING } PortState;

//global variables for thread shared memory
pthread_mutex_t ifaceMutex;
int n;
char **names;
unsigned char **neighbours;
unsigned char **interfaces;
unsigned char *bridgeId;
unsigned char ***macTable;
int *macIndices;
unsigned char **sentPackages;
int numPackages;
int packageIndex;
PortState *states;
time_t *timestamps;
int *socks;
time_t firstTcTime;
int hadTc;
int tca;
int rootPathCost;
unsigned char root[6];
unsigned char rootPriority, rootExtension;
unsigned short messageAge;

//global config variables
int helloTime;
int maxAge;
int forwardDelay;
int portCost;
unsigned char priority, extension;
int macTableSize;

int create_socket(char *device, int *sockfd, unsigned char *mac){
    int sock;
    struct ifreq ifr;
    struct sockaddr_ll sll;
    memset(&ifr, 0, sizeof(ifr));
    memset(&sll, 0, sizeof(sll));

    sock = socket (PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if(sock == 0) { printf("ERR: socket creation for device: %s\n", device); return -1; }
    strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
    if(ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    }else{
        perror("error on MAC address read");
        return -1;
    }
    if(ioctl(sock, SIOCGIFINDEX, &ifr) == -1) { printf(" ERR: ioctl failed for device: %s\n", device); return -1; }

    sll.sll_family      = AF_PACKET;
    sll.sll_ifindex     = ifr.ifr_ifindex;
    sll.sll_protocol    = htons(ETH_P_ALL);
    if(bind(sock, (struct sockaddr *) &sll, sizeof(sll)) == -1) { printf("ERR: bind failed for device: %s\n", device); return -1; }
    *sockfd = sock;
    return 0;
}

void generateSTP(unsigned char *packet, unsigned char *btype, unsigned char *bflags, unsigned char *port, int padding){
    unsigned char dst[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
    unsigned char len[2] = { 0x00, 0x26 };
    //logical link control
    unsigned char llc[3] = { 0x42, 0x42, 0x03 };
    //stp
    //protocol identifier
    unsigned char pid[2] = { 0x00, 0x00 };
    //version identifier
    unsigned char vid[1] = { 0x00 };

    //prepare variables for network byte order
    int netPath = htonl(rootPathCost);
    short netAge = messageAge;
    short netMax = maxAge;
    short netHlt = helloTime;
    short netFwd = forwardDelay;

    unsigned char *ptrs[] = {dst, bridgeId, len, llc, pid, vid, btype, bflags, &rootPriority, &rootExtension, root, (unsigned char *) &netPath, &priority, &extension, bridgeId, port, (unsigned char *) &netAge, (unsigned char *) &netMax, (unsigned char *) &netHlt, (unsigned char *) &netFwd};
    int sizes[] = {6, 6, 2, 3, 2, 1, 1, 1, 1, 1, 6, 4, 1, 1, 6, 2, 2, 2, 2, 2};
    int offset = 0;
    for(int i=0; i<sizeof(ptrs)/sizeof(unsigned char*); i++){
        memcpy(packet+offset, ptrs[i], sizes[i]);
        offset += sizes[i];
    }
}

void sendSTP(int index){
    pthread_mutex_lock(&ifaceMutex);
    //bpdu types
    //    unsigned char tcn[1] = { 0x80 };
    unsigned char cnf[1] = { 0x00 };

    //flags
    unsigned char tcf[1] = { 0x01 };
    unsigned char taf[1] = { 0xf0 };
    //stp identifiers
    unsigned char prt[2] = { 0x80, 0x01 };

    unsigned char packet[64];

    unsigned char flags[1] = { 0 };
    if(firstTcTime + forwardDelay > time(0))
        flags[0] += tcf[0];
    if(tca)
        flags[0] += taf[0];

    generateSTP(packet, cnf, flags, prt, 4);
    write(socks[index], packet, 64);
    pthread_mutex_unlock(&ifaceMutex);

}

//requires the ifaceMutex to be held
void sendTCN(int index){
    unsigned char dst[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
    unsigned char len[2] = { 0x00, 0x07 };
    unsigned char llc[3] = { 0x42, 0x42, 0x03 };

    unsigned char proto[2] = { 0, 0 };
    unsigned char version[1] = { 0 };
    unsigned char type[1] = { 0x80 };

    unsigned char packet[64];
    unsigned char *ptrs[] = {dst, bridgeId, len, llc, proto, version, type};
    int sizes[] = {6, 6, 2, 3, 2, 1, 3};
    int offset = 0;
    for(int i=0; i<sizeof(ptrs)/sizeof(unsigned char*); i++){
        memcpy(packet+offset, ptrs[i], sizes[i]);
        offset += sizes[i];
    }
    for(; offset<64; offset++)
        packet[offset] = 0;

    write(socks[index], packet, 64);
    printf("sending tcn\n");
}

int compareBridges(unsigned char aPrio, unsigned char aExt, unsigned char *aMac, unsigned char bPrio, unsigned char bExt, unsigned char *bMac){
    if(aPrio == bPrio)
        if(aExt == bExt)
            return memcmp(aMac, bMac, 6);
        else
            return aExt - bExt;
    else
        return aPrio - bPrio;
}

void updatePortStates(int currentIndex, unsigned char rPriority, unsigned char rExtension, unsigned char *rMac, unsigned int pathCost, unsigned char age, unsigned char bPriority, unsigned char bExtension){
    pthread_mutex_lock(&ifaceMutex);

    //check for a root change
    if(compareBridges(rPriority, rExtension, rMac, rootPriority, rootExtension, root) < 0 ||
            (compareBridges(rPriority, rExtension, rMac, rootPriority, rootExtension, root) == 0 && pathCost + portCost < rootPathCost)){
        memcpy(root, rMac, 6);
        rootPriority = rPriority;
        rootExtension = rExtension;
        rootPathCost = pathCost + portCost;
        messageAge = age+1;

        for(int i=0; i<n; i++)
            if(states[i] == ROOT)
                states[i] = DEDICATED;

        states[currentIndex] = ROOT;
        sendTCN(currentIndex);
    }

    //check if we would be the correct root
    if(compareBridges(priority, extension, bridgeId, rootPriority, rootExtension, root) < 0){
        memcpy(root, bridgeId, 6);
        rootPriority = priority;
        rootExtension = extension;
        rootPathCost = 0;

        for(int i=0; i<n; i++)
            states[i] = DEDICATED;
    }

    //if a port is in the BLOCKING state but shouldn't be, change it
    if(states[currentIndex] == BLOCKING){
        //if our root is the correct one, set the port to dedicated
        if(compareBridges(rootPriority, rootExtension, root, rPriority, rExtension, rMac) < 0)
            states[currentIndex] = DEDICATED;

        //if we have the same root, but have should be preferred, change to DEDICATED
        if(compareBridges(rootPriority, rootExtension, root, rPriority, rExtension, rMac) == 0 &&
            (rootPathCost < pathCost || (rootPathCost == pathCost && compareBridges(priority, extension, bridgeId, bPriority, bExtension, neighbours[currentIndex]) < 0)))
                states[currentIndex] = DEDICATED;
    }

    //if a port should is DEDICATED but shouldn't be, change it
    //only possibility should be same root different path cost
    if(states[currentIndex] == DEDICATED){
        //only change if the neighbour has the same root (smaller root is handled by root change, larger root is ignored -> stay DEDICATED)
        //note that here we have a rootPathCost == pathCost (without adding the portcost). This is because here we are interested in whether or not they have the same distance to the root.
        if(compareBridges(rPriority, rExtension, rMac, rootPriority, rootExtension, root) == 0)
            //even then only change to BLOCKING if they have a shorter path or should be preferred
            if(rootPathCost > pathCost || (rootPathCost == pathCost && compareBridges(priority, extension, bridgeId, bPriority, bExtension, neighbours[currentIndex]) > 0))
                states[currentIndex] = BLOCKING;
    }

    pthread_mutex_unlock(&ifaceMutex);
}

void processPacket(u_char *user, const struct pcap_pkthdr *header, const u_char *bytes){
    struct ethhdr *ethh = (struct ethhdr*) bytes;

    int currentIndex = *(int *) user;
   
    pthread_mutex_lock(&ifaceMutex);
    int hasRoot = 0;
    for(int i=0; i<n; i++){
        if(timestamps[i] + forwardDelay < time(0))
            states[i] = DEDICATED;
        if(states[i] == ROOT)
            hasRoot = 1;
    }
    //this is a workaround for the root reset issue
    //this problem would never resolve on its own, but this workaround fixes it after one helloTime
    int rootNonNull = 0;
    for(int i=0; i<6; i++)
        if(root[i] != 0)
            rootNonNull = 1;

    if(!hasRoot || !rootNonNull){
        memcpy(root, bridgeId, 6);
        rootPriority = priority;
        rootExtension = extension;
        rootPathCost = 0;
        for(int i=0; i<n; i++)
            states[i] = DEDICATED;
    }

    pthread_mutex_unlock(&ifaceMutex);

    char buffer[17];
    sprintf(buffer,"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", ethh->h_dest[0], ethh->h_dest[1], ethh->h_dest[2], ethh->h_dest[3], ethh->h_dest[4], ethh->h_dest[5]);
    if(strncmp(buffer, "01:80:C2:00:00:00", 17) == 0){
        if(memcmp(ethh->h_source, bridgeId, 6) == 0){
            return; 
        }

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

        pthread_mutex_lock(&ifaceMutex);
        unsigned char tcSet = *payload++;
        if(!hadTc && tcSet){
            firstTcTime = time(0);
        }
        hadTc = tcSet;
        psize--;
        pthread_mutex_unlock(&ifaceMutex);

        //root identifier
        //bridge priority is the next 2 bytes
        //one for priority, one for id extension
        unsigned char rPriority = *payload++;
        unsigned char rExtension = *payload++;
        psize-=2;
        //next 6 bytes is the root bridge id
        unsigned char rootMac[6];
        memcpy(rootMac, payload, 6);
        payload+=6;
        psize-=6;

        //root path cost (4 bytes)
        int pathCost = ntohl(*(int*) payload);
        payload+=4;
        psize-=4;

        //bridge identifier 
        //priority
        unsigned char bPriority = *payload++;
        unsigned char bExtension = *payload++;
        psize-=2;
        //next 6 bytes is the bridge mac address
        unsigned char bridgeMac[6];
        memcpy(bridgeMac, payload, 6);
        payload+=6;
        psize-=6;


        //next 2 bytes are port identifier
        payload+=2;
        psize-=2;

        //next 2 bytes is message age
        short age = ntohs(*(short *) payload);
        
        payload+=2;
        psize-=2;

        //check for tcn packages
        int nonNull = 0;
        for(int i=0; i<6; i++){
            if(rootMac[i] != 0 || bridgeMac[i] != 0){
                nonNull = 1;
                break;
            }
        }
        //tcn package
        if(!nonNull){
            tca = 1;
            return;
        }else{
            tca = 0;
            pthread_mutex_lock(&ifaceMutex);
            memcpy(neighbours[currentIndex], payload, 6);
            pthread_mutex_unlock(&ifaceMutex);
        }
    
        timestamps[currentIndex] = time(0);
        updatePortStates(currentIndex, rPriority, rExtension, rootMac, pathCost, age, bPriority, bExtension);
    }else{
        //handle non-stp packets here
        //skip packets originating from this interface
        if(memcmp(ethh->h_source, interfaces[currentIndex], 6) == 0){
            return;
        }

        //skip packets sent by another interface listener
        for(int i=0; i<numPackages; i++)
            if(sentPackages[i] != 0 && memcmp(sentPackages[i], bytes, header->len) == 0)
                return;

        memcpy(sentPackages[packageIndex], bytes, header->len);
        packageIndex = (packageIndex+1)%numPackages;

        //add src mac to mac table
        int found = 0;
        for(int i=0; i<macTableSize; i++){
            if(macTable[currentIndex][i] == NULL)
                continue;
            if(memcmp(macTable[currentIndex][i], ethh->h_source, 6) == 0){
                found = 1;
                break;
            }
        }
        if(!found){
            memcpy(macTable[currentIndex][macIndices[currentIndex]], ethh->h_source, 6);
            macIndices[currentIndex] = (macIndices[currentIndex] + 1)%macTableSize;
        }

        //check if the receiving interface is the intended target
        if(memcmp(ethh->h_dest, interfaces[currentIndex], 6) == 0)
            return;
        
        //find right interface to send it on
        int targetIndex = -1;
        for(int i=0; i<n && targetIndex < 0; i++){
            if(i==currentIndex)
                continue;
            for(int j=0; j<macTableSize; j++){
                if(memcmp(macTable[i][j], ethh->h_dest, 6) == 0){
                    targetIndex = i;
                    break;
                }
            }
        }
        
        if(targetIndex < 0 || ethh->h_dest[0] & 1){
            //if the packet was received on a blocking interface it is travelling sideways
            //and needs to be stopped
            if(states[currentIndex] == BLOCKING)
                return;

            //we don't know where to send it
            //this sends the packet via all dedicated ports and the root if it is going up (coming from a dedicated port)
            //and all dedicated ports if it is going down (coming from the root port)
            for(int i=0; i<n; i++)
                if(i!=currentIndex && states[i] != BLOCKING)
                    write(socks[i], bytes, header->len);
        }else{
            write(socks[targetIndex], bytes, header->len);
        }
    }
}

void dumpMacTable(){
    for(int i=0; i<n; i++){
        printf("interface %s:\n", names[i]);
        for(int j=0; j<macTableSize; j++){
            int isnull = 1;
            for(int k=0; k<6; k++){
                if(macTable[i][j][k] != 0){
                    isnull = 0;
                    break;
                }
            }
            if(isnull)
                continue;

            printf("%.2X", macTable[i][j][0]);
            for(int k=1; k<6; k++)
                printf(":%.2X", macTable[i][j][k]);
            printf("\n");
        }
        printf("\n");
    }
}

void *senderThread(void *arg){
    while(1){
        for(int i=0; i<n; i++){
            pthread_mutex_lock(&ifaceMutex);
            if(socks[i] < 0){
                printf("interface %s is DISCONNECTED, not sending\n", names[i]);
                pthread_mutex_unlock(&ifaceMutex);
                continue;
            }
            pthread_mutex_unlock(&ifaceMutex);
            switch(states[i]){
                case DEDICATED:
                    printf("interface %s is DEDICATED, sending packet\n", names[i]);
                    sendSTP(i);
                    break;
                case BLOCKING:
                    printf("interface %s is BLOCKING, not sending\n", names[i]);
                    break;
                case ROOT:
                    printf("interface %s is ROOT, not sending\n", names[i]);
                    break;
            }
        }
        sleep(2);
    }
}

void *interFaceThread(void *arg){
    int currentIndex = *(int *) arg;
    char err[PCAP_ERRBUF_SIZE];
    pcap_t *captureHandle = pcap_open_live(names[currentIndex], 65536, 1, 0, err);
    if(!captureHandle){
        fprintf(stderr, "Could not start capture on device %s\n", names[currentIndex]);
        return NULL;
    }
    printf("Capture handle on interface %s created\n", names[currentIndex]);
    while(1){
        pcap_loop(captureHandle, -1, processPacket, (u_char *) &currentIndex);
        
        pcap_close(captureHandle);
        pthread_mutex_lock(&ifaceMutex);
        close(socks[currentIndex]);
        socks[currentIndex] = -1;
        pthread_mutex_unlock(&ifaceMutex);
        printf("capture handle and socket on interface %s closed\n", names[currentIndex]);

        printf("trying to find interface again\n");
        captureHandle = NULL;
        while(captureHandle == NULL){
            sleep(1);

            pcap_if_t *alldevs;
            if(pcap_findalldevs(&alldevs, err)){
                fprintf(stderr, "Could not find all interfaces, error code: %s\n", err);
                exit(-1);
            }
            pcap_if_t *dev = alldevs;
            for(; dev != NULL && strcmp(dev->name, names[currentIndex]); dev = dev->next)
                ;
            if(dev != NULL){
                pthread_mutex_lock(&ifaceMutex);
                if(create_socket(names[currentIndex], socks+currentIndex, interfaces[currentIndex]) < 0){
                    perror("could not create socket");
                    exit(-1);
                }
                pthread_mutex_unlock(&ifaceMutex);

                captureHandle = pcap_open_live(names[currentIndex], 65536, 1, 0, err);
            }
        }
        printf("interface %s found, restarting\n", names[currentIndex]);
    }

    return NULL;
}

int main(int argc, char ** argv){
    //default values
    helloTime = 2;
    maxAge = 20;
    forwardDelay = 15;
    portCost = 20000;
    priority = 0x80;
    extension = 0x01;
    macTableSize = 30;
    numPackages = 30;
    packageIndex = 0;

    //configure globals
    int c=1;
    for(;c<argc;c++){
        if(argv[c][0] != '-')
            break;
        if(strcmp(argv[c], "--") == 0){
            c++;
            break;
        }else if(strcmp(argv[c], "-p") == 0){
            priority = atoi(argv[++c]);
        }else if(strcmp(argv[c], "-e") == 0){
            extension = atoi(argv[++c]);
        }else if(strcmp(argv[c], "-ms") == 0){
            macTableSize = atoi(argv[++c]);
        }
    }

    printf("configuration variables:\nhelloTime:\t%d\nmaxAge:\t%d\nforwardDelay:\t%d\nportCost:\t%d\npriority:\t%d\nextension:\t%d\n", helloTime, maxAge, forwardDelay, portCost, priority, extension);

    //initialize shared variables
    n = argc - c;
    names = (char **) calloc(n, sizeof(char*));
    neighbours = (unsigned char **) calloc(n, sizeof(char*));
    interfaces = (unsigned char **) calloc(n, sizeof(char*));
    macTable = (unsigned char ***) calloc(n, sizeof(char**));
    macIndices = (int *) calloc(n, sizeof(int));
    sentPackages = (unsigned char **) calloc(numPackages, sizeof(char*));
    states = (PortState *) calloc(n, sizeof(PortState));
    timestamps = (time_t *) calloc(n, sizeof(time_t));
    socks = (int *) calloc(n, sizeof(int));
    for(int i=0; i<n; i++){
        names[i] = (char *) calloc(strlen(argv[c+i]), sizeof(char));
        strcpy(names[i], argv[c+i]);
        neighbours[i] = (unsigned char *) calloc(6, sizeof(char));
        interfaces[i] = (unsigned char *) calloc(6, sizeof(char));
        macTable[i] = (unsigned char **) calloc(30, sizeof(char **));
        for(int j=0; j<macTableSize; j++)
            macTable[i][j] = (unsigned char *) calloc(6,sizeof(char));
        for(int j=0; j<6; j++)
            neighbours[i][j] = 0xff;
        timestamps[i] = 0;
        states[i] = DEDICATED;
    }
    for(int i=0; i<numPackages; i++)
        sentPackages[i] = (unsigned char*) calloc(65536, sizeof(char *));

    //initialize mutex
    pthread_mutex_init(&ifaceMutex, NULL);

    pthread_t ifaceThreads[n];
    for(int i=0; i<n; i++){
        if(create_socket(argv[c+i], socks+i, interfaces[i]) < 0){
            perror("could not create socket"); return -1;
        }
    }
    bridgeId = interfaces[0];

    //configure default root
    pthread_mutex_lock(&ifaceMutex);
    memcpy(root, bridgeId, 6);
    firstTcTime = time(0);
    rootPriority = priority;
    rootExtension = extension;
    messageAge = 0;
    rootPathCost = 0;
    pthread_mutex_unlock(&ifaceMutex);
    
    int indices[n];
    for(int i=0; i<n; i++){
        indices[i] = i;

        //the indices solution is not pretty, but it keeps the loop from running away with i
        //(which would result in segfaults)
        if(pthread_create(&ifaceThreads[i], NULL, &interFaceThread, &indices[i]) < 0){
            perror("Could not create interface thread");
            exit(-1);
        }else{
            printf("Created interface thread for interface %s\n", names[i]);
        }
    }

    //create sender
    pthread_t sender;
    if(pthread_create(&sender, NULL, senderThread, NULL) < 0){
        perror("Could not create sender thread");
        exit(-1);
    }

    for(int i=0; i<n; i++)
        pthread_join(ifaceThreads[i], NULL);
    pthread_join(sender, NULL);

    printf("Main done waiting\n");

    //free shared variables
    //note that the pthread_join will never stop blocking, but if all pcap_loops are cancelled we can free this
    //and it's good form
    for(int i=0; i<n; i++){
        free(names[i]);
        free(neighbours[i]);
        free(interfaces[i]);
        for(int j=0; j<macTableSize; j++)
            free(macTable[i][j]);
        free(macTable[i]);
    }

    for(int i=0; i<numPackages; i++)
        free(sentPackages[i]);

    free(sentPackages);
    free(names);
    free(neighbours);
    free(interfaces);
    free(macTable);
    free(states);
    free(timestamps);
    free(socks);
    free(ifaceThreads);
}
