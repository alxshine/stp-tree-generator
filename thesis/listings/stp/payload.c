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
unsigned short rPriority;
//actual priority is 4 bits
rPriority = *((short*)payload);
payload+=2;
psize-=2;
//next 6 bytes is the root bridge id
Mac rootMac(payload);
payload+=6;
psize-=6;
