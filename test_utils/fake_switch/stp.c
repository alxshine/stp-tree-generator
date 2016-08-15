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
PortState *states;
time_t *timestamps;
int *socks;
time_t firstTcTime;
int hadTc;
int rootPathCost;
unsigned char root[6];
unsigned char rootPriority, rootExtension;
unsigned char bridgeId[6];
unsigned short messageAge;

//global config variables
int helloTime;
int maxAge;
int forwardDelay;
int portCost;
unsigned char priority, extension;

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

void generateSTP(unsigned char *packet, unsigned char *btype, unsigned char *bflags, unsigned char *port, unsigned char *max, unsigned char *hlt, unsigned char *fwd, int padding){
    unsigned char dst[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
    unsigned char len[2] = { 0x00, 0x26 };
    //logical link control
    unsigned char llc[3] = { 0x42, 0x42, 0x03 };
    //stp
    //protocol identifier
    unsigned char pid[2] = { 0x00, 0x00 };
    //version identifier
    unsigned char vid[1] = { 0x00 };

    unsigned char *ptrs[] = {dst, bridgeId, len, llc, pid, vid, btype, bflags, &rootPriority, &rootExtension, root, (unsigned char *) &rootPathCost, &priority, &extension, bridgeId, port, (unsigned char *) &messageAge, max, hlt, fwd};
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
    //unsigned char taf[1] = { 0xf0 };
    //stp identifiers
    unsigned char prt[2] = { 0x80, 0x01 };
    unsigned char mxa[2] = { 0x14, 0x00 };
    unsigned char hlt[2] = { 0x02, 0x00 };
    unsigned char fwd[2] = { 0x0f, 0x00 };

    unsigned char packet[64];

    unsigned char flags[1] = { 0 };
    if(firstTcTime + forwardDelay > time(0))
        flags[0] += tcf[0];
    
    generateSTP(packet, cnf, flags, prt, mxa, hlt, fwd, 4);
    write(socks[index], packet, 64);
    timestamps[index] = time(0);     
    pthread_mutex_unlock(&ifaceMutex);

}

void processPacket(u_char *user, const struct pcap_pkthdr *header, const u_char *bytes){
    struct ethhdr *ethh = (struct ethhdr*) bytes;

    int currentIndex = *(int *) user;
   

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

        //root identifier
        //bridge priority is the next 2 bytes
        //one for priority, one for id extension
        //together they function as a priority, so we can just store them together in a short
        unsigned char rPriority = *payload++;
        unsigned char rExtension = *payload++;
        psize-=2;
        //next 6 bytes is the root bridge id
        unsigned char rootMac[6];
        memcpy(rootMac, payload, 6);
        payload+=6;
        psize-=6;

        //root path cost (4 bytes)
        int pathCost = *(int*) payload;
        payload+=4;
        psize-=4;

        //bridge identifier (but we don't care about that, so skip it)
        //priority
        unsigned char bPriority = *payload++;
        unsigned char bExtension = *payload++;
        psize-=2;
        //next 6 bytes is the bridge mac address
        memcpy(neighbours[currentIndex], payload, 6);
        pthread_mutex_unlock(&ifaceMutex);
        payload+=6;
        psize-=6;


        //next 2 bytes are port identifier
        payload+=2;
        psize-=2;

        //next 2 bytes is message age
        short age = *(short *) payload;

        //check for a root change
        if(rPriority < rootPriority ||
                (rPriority == rootPriority && rExtension < rootExtension) ||
                (rPriority == rootPriority && rExtension == extension && memcmp(rootMac, root, 6) < 0) ||
                (rPriority == rootPriority && rExtension == extension && memcmp(rootMac, root, 6) == 0 && pathCost + portCost > rootPathCost)){
            memcpy(root, rootMac, 6);
            rootPriority = rPriority;
            rootExtension = rExtension;
            rootPathCost = pathCost + portCost;
            messageAge = age;

            for(int i=0; i<n; i++)
                if(states[i] == ROOT)
                    states[i] = DEDICATED;

            states[currentIndex] = ROOT;
        }

        //check if we would be the correct root
        if(priority < rootPriority ||
                (priority == rootPriority && extension < rootExtension) ||
                (priority == rootPriority && extension == rootExtension && memcmp(bridgeId, root, 6) < 0)){
            memcpy(root, bridgeId, 6);
            rootPriority = priority;
            rootExtension = extension;
            rootPathCost = pathCost;

            for(int i=0; i<n; i++)
                states[i] = DEDICATED;
        }
 
        //if a port is in the BLOCKING state but shouldn't be, send an STP packet on it
        if(states[currentIndex] == BLOCKING && 
                (bPriority > priority || (bPriority == priority && bExtension > extension) || 
                 (bPriority == priority && bExtension == extension && memcmp(neighbours[currentIndex], bridgeId, 6) > 0))){
            sendSTP(currentIndex);
            states[currentIndex] = DEDICATED;
        }

        //if a port should be BLOCKING but isn't, make it
        if(states[currentIndex] == DEDICATED && (bPriority < priority || 
                (bPriority == priority && bExtension < extension) ||
                 (bPriority == priority && bExtension == extension && memcmp(neighbours[currentIndex], bridgeId, 6) < 0))){
            states[currentIndex] = BLOCKING;
        }

        payload+=2;
        psize-=2;
    
        pthread_mutex_unlock(&ifaceMutex);
    }

}

void *senderThread(void *arg){
    while(1){
        for(int i=0; i<n; i++){
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
    pthread_mutex_lock(&ifaceMutex);
    pcap_t *captureHandle = pcap_open_live(names[currentIndex], 65536, 1, 0, err);
    if(!captureHandle){
        fprintf(stdout, "Could not start capture on device %s\n", names[0]);
        return NULL;
    }
    printf("Capture handle on interface %s created\n", names[currentIndex]);
    pthread_mutex_unlock(&ifaceMutex);
    pcap_loop(captureHandle, -1, processPacket, (u_char *) &currentIndex);

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
        }
    }

    printf("configuration variables:\nhelloTime:\t%d\nmaxAge:\t%d\nforwardDelay:\t%d\nportCost:\t%d\npriority:\t%d\nextension:\t%d\n", helloTime, maxAge, forwardDelay, portCost, priority, extension);

    //initialize shared variables
    n = argc - c;
    names = (char **) calloc(n, sizeof(char*));
    neighbours = (unsigned char **) calloc(n, sizeof(char*));
    states = (PortState *) calloc(n, sizeof(PortState));
    timestamps = (time_t *) calloc(n, sizeof(time_t));
    socks = (int *) calloc(n, sizeof(int));
    for(int i=0; i<n; i++){
        names[i] = (char *) calloc(strlen(argv[c+i]), sizeof(char));
        strcpy(names[i], argv[c+i]);
        neighbours[i] = (unsigned char *) calloc(6, sizeof(char));
        for(int j=0; j<6; j++)
            neighbours[i][j] = 0xff;
        timestamps[i] = 0;
        states[i] = DEDICATED;
    }

    //initialize mutex
    pthread_mutex_init(&ifaceMutex, NULL);

    pthread_t ifaceThreads[n];
    for(int i=0; i<n; i++){
        if(create_socket(argv[c+i], socks+i, bridgeId) < 0){
            perror("could not create socket"); return -1;
        }
    }

    //configure default root
    pthread_mutex_lock(&ifaceMutex);
    memcpy(root, bridgeId, 6);
    firstTcTime = time(0);
    rootPriority = priority;
    rootExtension = extension;
    messageAge = 0;pthread_mutex_unlock(&ifaceMutex);

    int indices[n];
    for(int i=0; i<n; i++){
        indices[i] = i;

        //the indices solution is not pretty, but it keeps the loop from running away with i
        //(resulting in segfaults)
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
    }
    free(names);
    free(neighbours);
    free(states);
    free(timestamps);
    free(socks);
    free(ifaceThreads);
}
