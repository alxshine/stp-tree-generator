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
