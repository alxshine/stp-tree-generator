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
