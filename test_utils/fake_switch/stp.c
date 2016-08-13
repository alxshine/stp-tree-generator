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
unsigned char bridgeId[6];
pthread_t *ifaceThreads;

//global config variables
int helloTime;
int maxAge;
int forwardDelay;
int portCost;

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

void generateSTP(unsigned char *packet, unsigned char *src, unsigned char *btype, unsigned char *bflags, unsigned char *prio, unsigned char *ext, unsigned char *cost, unsigned char *port, unsigned char *age, unsigned char *max, unsigned char *hlt, unsigned char *fwd, int padding){
    unsigned char dst[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
    unsigned char len[2] = { 0x00, 0x26 };
    //logical link control
    unsigned char llc[3] = { 0x42, 0x42, 0x03 };
    //stp
    //protocol identifier
    unsigned char pid[2] = { 0x00, 0x00 };
    //version identifier
    unsigned char vid[1] = { 0x00 };

    unsigned char *ptrs[] = {dst, src, len, llc, pid, vid, btype, bflags, prio, ext, src, cost, prio, ext, src, port, age, max, hlt, fwd};
    int sizes[] = {6, 6, 2, 3, 2, 1, 1, 1, 1, 1, 6, 4, 1, 1, 6, 2, 2, 2, 2, 2};
    int offset = 0;
    for(int i=0; i<sizeof(ptrs)/sizeof(unsigned char*); i++){
        memcpy(packet+offset, ptrs[i], sizes[i]);
        offset += sizes[i];
    }
}

void processPacket(u_char *user, const struct pcap_pkthdr *header, const u_char *bytes){
    struct ethhdr *ethh = (struct ethhdr*) bytes;

    int currentIndex = *(int *) user;
   
    if(memcmp(ethh->h_source, bridgeId, 6) == 0){
        return; 
    } 

    char buffer[17];
    sprintf(buffer,"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", ethh->h_dest[0], ethh->h_dest[1], ethh->h_dest[2], ethh->h_dest[3], ethh->h_dest[4], ethh->h_dest[5]);
    if(strncmp(buffer, "01:80:C2:00:00:00", 17) != 0){
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
        pthread_mutex_unlock(&ifaceMutex);
        psize--;

        //root identifier
        //bridge priority is the next 2 bytes
//        unsigned short rPriority;
        //actual priority is 4 bits
//        rPriority = *((short*)payload);
        payload+=2;
        psize-=2;
        //next 6 bytes is the root bridge id
//        unsigned char rootMac[6];
//        memcpy(rootMac, payload, 6);
        payload+=6;
        psize-=6;

        //root path cost (4 bytes)
//        int pathCost = *(int*) payload;
        payload+=4;
        psize-=4;

        //bridge identifier (but we don't care about that, so skip it)
        ////bridge priority is the next 2 bytes
    //    unsigned short bPriority;
    //    bPriority = *((short*)payload);
        payload+=2;
        psize-=2;
        //next 6 bytes is the bridge mac address
        pthread_mutex_lock(&ifaceMutex);
        memcpy(neighbours[currentIndex], payload, 6);
        pthread_mutex_unlock(&ifaceMutex);
        payload+=6;
        psize-=6;
    //    if(rootPathCost > pathCost){
    //        for(int i=0; i<n; i++){
    //            if(states[i] == ROOT && currentIndex != i){
    //                if(pathCost + portCost < rootPathCost)
    //                    states[i] = DEDICATED;
    //                else
    //                    states[i] = BLOCKING;
    //            }
    //        }
    //    }

        //next 2 bytes are port identifier
        payload+=2;
        psize-=2;

        //next 2 bytes is message age
    //    short messageAge;
    //    messageAge = *((short*)payload);
        payload+=2;
        psize-=2;
    }


    
}

void *senderThread(void *arg){
    int currentIndex = *(int *) arg;
    pthread_mutex_unlock(&ifaceMutex);
    while(1){
        pthread_mutex_lock(&ifaceMutex);
        //bpdu types
        //unsigned char tcn[1] = { 0x80 };
        unsigned char cnf[1] = { 0x00 };

        //flags
        unsigned char tcf[1] = { 0x01 };
        //unsigned char taf[1] = { 0xf0 };
        //stp identifiers
        //priority (at default values)
        unsigned char pri[1] = { 0x00 };
        unsigned char ext[1] = { 0x00 };
        unsigned char pth[4] = { 0x00, 0x00, 0x00, 0x00 };

        unsigned char prt[2] = { 0x80, 0x01 };
        unsigned char age[2] = { 0x00, 0x00 };
        unsigned char mxa[2] = { 0x14, 0x00 };
        unsigned char hlt[2] = { 0x02, 0x00 };
        unsigned char fwd[2] = { 0x0f, 0x00 };

        unsigned char packet[64];

        unsigned char flags[1] = { 0 };
        if(firstTcTime + forwardDelay > time(0))
            flags[0] += tcf[0];
        
        generateSTP(packet, bridgeId, cnf, tcf, pri, ext, pth, prt, age, mxa, hlt, fwd, 4);
        printf("Sending packet on interface %s\n", names[currentIndex]);
        write(socks[currentIndex], packet, 64);
        timestamps[currentIndex] = time(0);     
        pthread_mutex_unlock(&ifaceMutex);
        
        sleep(2);
    }
}

void *interFaceThread(void *arg){
    int currentIndex = *(int *) arg;
    pthread_t sender;
    if(pthread_create(&sender, NULL, senderThread, arg) < 0){
        perror("Could not create sender thread");
        exit(-1);
    }
    printf("Created sender thread for interface %s\n", names[currentIndex]);

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
    pthread_join(sender, NULL);

    return NULL;
}

int main(int argc, char ** argv){
    //default values
    helloTime = 2;
    maxAge = 20;
    forwardDelay = 15;
    portCost = 20000;

    //configure globals
    int c=1;
    for(;c<argc;c++){
        if(argv[c][0] != '-')
            break;
        if(strcmp(argv[c], "--") == 0){
            c++;
            break;
        }
    }

    //initialize shared variables
    n = argc - c;
    names = (char **) calloc(n, sizeof(char*));
    neighbours = (unsigned char **) calloc(n, sizeof(char*));
    states = (PortState *) calloc(n, sizeof(PortState));
    timestamps = (time_t *) calloc(n, sizeof(time_t));
    socks = (int *) calloc(n, sizeof(int));
    ifaceThreads = (pthread_t *) calloc(n, sizeof(pthread_t));
    for(int i=0; i<n; i++){
        names[i] = (char *) calloc(strlen(argv[c+i]), sizeof(char));
        strcpy(names[i], argv[c+i]);
        neighbours[i] = (unsigned char *) calloc(6, sizeof(char));
        for(int j=0; j<6; j++)
            neighbours[i][j] = 0xff;
        timestamps[i] = 0;
        firstTcTime = time(0);
    }
    //initialize mutex
    pthread_mutex_init(&ifaceMutex, NULL);

    int indices[n];
    for(int i=0; i<n; i++){
        indices[i] = i;

        if(create_socket(argv[c+i], socks+i, bridgeId) < 0){
            perror("could not create socket"); return -1;
        }
        //the indices solution is not pretty, but it keeps the loop from running away with i
        //(resulting in segfaults)
        if(pthread_create(&ifaceThreads[i], NULL, &interFaceThread, &indices[i]) < 0){
            perror("Could not create interface thread");
            exit(-1);
        }else{
            printf("Created interface thread for interface %s\n", names[i]);
        }
    }
    for(int i=0; i<n; i++)
        pthread_join(ifaceThreads[i], NULL);

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
