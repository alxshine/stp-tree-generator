std::vector<SpanningTree> indivTrees = std::vector<SpanningTree>();
for(auto mapIt = clientData.begin(); mapIt != clientData.end(); ++mapIt){
    auto vec = mapIt->second;
    std::sort(vec.begin(), vec.end(), [](Bridge a, Bridge b) {return a.getMessageAge() < b.getMessageAge();});
    indivTrees.push_back(treeHelper(vec.begin(), vec.end()));
}

//now combine these trees
std::vector<SpanningTree> ret;
for(auto indivIt = indivTrees.begin(); indivIt != indivTrees.end(); ++indivIt){
    bool contained = false;
    for(auto treeIt = ret.begin(); treeIt != ret.end(); ++treeIt){
        if(treeIt->containsRoot(*indivIt) && *treeIt != *indivIt){
            *treeIt = *treeIt + *indivIt;
            contained = true;
            break;
        }
    }
    if(!contained)
        ret.push_back(*indivIt);
}

