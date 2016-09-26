//handle network changes
if(rootContained){
    if(oldHopMa >= 0){
        //both contained
        if(oldHopMa != firstHop.getMessageAge()){
            //upstream changes
            clearAndAdd(firstHop, root);
        }else{
            //everything is as it was
            //if something changed upstream a tc flag will be sent
            //->reset
            if(tc && !hadTC)
                clearAndAdd(firstHop, root);
        }
    }else{
        //first hop not contained
        //this means that the node was plugged in on a different bridge
        //(most likely)
        //we don't know how long we were disconnected, so reregister the client
        if(!noConnect)
            client->regServer();
        clearAndAdd(firstHop, root);
    }
}else{
    //root not contained
    if(oldHopMa >= 0){
        //if the first hop was contained this means there were some changes upstream
        //the root moving away means we assume network growth
        if(oldHopMa < firstHop.getMessageAge()){
            std::cout << "root moved away\n";
            int maDiff = oldHopMa - firstHop.getMessageAge();
            //this means we have to increase every message age and add the new root
            for(Bridge &b : bridges)
                b.setMessageAge(b.getMessageAge() + maDiff);
            bridges.push_back(root);
        }else{
            clearAndAdd(firstHop, root);
        }
    }else{
        //entirely new setup
        clearAndAdd(firstHop, root);
    }
}
