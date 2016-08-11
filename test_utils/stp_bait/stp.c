
/*---------- The Linux Channel ------------*/

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

typedef unsigned char  BYTE;    /* 8-bit   */

enum _Boolean_ { FALSE=0, TRUE=1};

int create_socket(char *device, int *sockfd, BYTE *mac){
    int sock;
    struct ifreq ifr;
    struct sockaddr_ll sll;
    memset(&ifr, 0, sizeof(ifr));
    memset(&sll, 0, sizeof(sll));

    sock = socket (PF_PACKET,SOCK_RAW,htons(ETH_P_ALL));

    if(sock == 0) { printf("ERR: socket creation for device: %s\n", device); return FALSE; }
    strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
    if(ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    }else{
        perror("error on MAC address read");
        return -1;
    }
    if(ioctl(sock, SIOCGIFINDEX, &ifr) == -1) { printf(" ERR: ioctl failed for device: %s\n", device); return FALSE; }

    sll.sll_family      = AF_PACKET;
    sll.sll_ifindex     = ifr.ifr_ifindex;
    sll.sll_protocol    = htons(ETH_P_ALL);
    if(bind(sock, (struct sockaddr *) &sll, sizeof(sll)) == -1) { printf("ERR: bind failed for device: %s\n", device); return FALSE; }
    *sockfd = sock;
    return 0;
}

void generateSTP(BYTE *packet, BYTE *src, BYTE *btype, BYTE *bflags, BYTE *prio, BYTE *ext, BYTE *cost, BYTE *port, BYTE *age, BYTE *max, BYTE *hlt, BYTE *fwd, int padding){
    BYTE dst[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
    BYTE len[2] = { 0x00, 0x26 };
    //logical link control
    BYTE llc[3] = { 0x42, 0x42, 0x03 };
    //stp
    //protocol identifier
    BYTE pid[2] = { 0x00, 0x00 };
    //version identifier
    BYTE vid[1] = { 0x00 };

    BYTE *ptrs[] = {dst, src, len, llc, pid, vid, btype, bflags, prio, ext, src, cost, prio, ext, src, port, age, max, hlt, fwd};
    int sizes[] = {6, 6, 2, 3, 2, 1, 1, 1, 1, 1, 6, 4, 1, 1, 6, 2, 2, 2, 2, 2};
    int offset = 0;
    for(int i=0; i<sizeof(ptrs)/sizeof(BYTE*); i++){
        memcpy(packet+offset, ptrs[i], sizes[i]);
        offset += sizes[i];
    }
}

int main(int argc, char ** argv){   
    int sockfd=0; 
    BYTE src[6];
    if(create_socket(argv[1], &sockfd, src) < 0){ perror("could not create socket"); return -1;}

    //bpdu types
    BYTE tcn[1] = { 0x80 };
    BYTE cnf[1] = { 0x00 };

    //flags
    BYTE nfg[1] = { 0x00 };
    BYTE tcf[1] = { 0x01 };
    BYTE taf[1] = { 0xf0 };
    //stp identifiers
    //priority (at default values)
    BYTE pri[1] = { 0x00 };
    BYTE ext[1] = { 0x00 };
    BYTE pth[4] = { 0x00, 0x00, 0x00, 0x00 };

    BYTE prt[2] = { 0x80, 0x01 };
    BYTE age[2] = { 0x00, 0x00 };
    BYTE mxa[2] = { 0x14, 0x00 };
    BYTE hlt[2] = { 0x02, 0x00 };
    BYTE fwd[2] = { 0x0f, 0x00 };


    while(1){
        printf("What would you like to do?\n");
        printf("(1): Send TC Package\n");
        printf("(2): Send TCN Package\n");
        printf("(3): Send TCA Package\n");
        printf("(4): Send conf Package\n");

        BYTE packet[64];
        int val;
        scanf("%d", &val);
        switch(val){
        case 1:
            generateSTP(packet, src, cnf, tcf, pri, ext, pth, prt, age, mxa, hlt, fwd, 4);
            break;
        case 2:
            generateSTP(packet, src, tcn, tcf, pri, ext, pth, prt, age, mxa, hlt, fwd, 4);
            break;
        case 4:
            generateSTP(packet, src, cnf, taf, pri, ext, pth, prt, age, mxa, hlt, fwd, 4);
            break;
        case 5:
            generateSTP(packet, src, cnf, nfg, pri, ext, pth, prt, age, mxa, hlt, fwd, 4);
            break;
        }
        write(sockfd, packet, sizeof(packet));
    }
}
