//remove invalid entries before creating SpanningTree
//check every received vector against every other
for(auto mapIt1 = clientData.begin(); mapIt1 != clientData.end(); ++mapIt1){
    std::vector<Bridge> vec1 = mapIt1->second;
    for(auto mapIt2 = clientData.begin(); mapIt2 != clientData.end(); ++mapIt2){
        if(mapIt1 == mapIt2)
            continue;
        std::vector<Bridge> vec2 = mapIt2->second;
        for(auto vecIt1 = vec1.begin(); vecIt1 != vec1.end(); ++vecIt1){
            for(auto vecIt2 = vec2.begin(); vecIt2 != vec2.end(); ++vecIt2){
                //check every bridge against every other
                //if one has a smaller message age, it is incorrect and needs to be removed
                //this might overlook some errors if the number of clients is insufficient
                if(vecIt1->getMessageAge() < vecIt2->getMessageAge()){
                    //remove bridge
                    vecIt1=vec1.erase(vecIt1);
                    //decrement vecIt1 to have it at the correct position in the next loop iteration
                    --vecIt1;
                    //continue with outer loop (vec1 iteration)
                    break;
                }else if(vecIt2->getMessageAge() < vecIt1->getMessageAge()){
                    vecIt2=vec2.erase(vecIt2);
                    --vecIt2;
                    //no break needed -> next step is inner loop (vec2 iteration)
                }
            }
        }
    }
}
