//generate Bridge objects
Bridge root(rootMac, rPriority, 0);
Bridge firstHop(bridgeMac, bPriority, messageAge);

//check if the two nodes are contained
int rootContained = 0, oldHopMa = -1;
for(Bridge b : bridges){
    if(b == root)
        rootContained = 1;
    if(b == firstHop)
        oldHopMa = b.getMessageAge();
}
