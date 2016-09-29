std::string SpanningTree::toTikz(double lowerX, double upperX, int y, int yStep, int oldMessageAge, int index) const {
    return toTikzHelper(lowerX, upperX, y, yStep, oldMessageAge, index).first;
}

std::pair<std::string, int> SpanningTree::toTikzHelper(double lowerX, double upperX, int y, int yStep, int oldMessageAge, int index) const {
    //this helper is needed because the previously used child index calculation was faulty if the algorithm was adding empty nodes
    std::string retString;
    int retInt = index;

    double dist = upperX - lowerX;
    double x = lowerX + dist/2;

    if(root.getMessageAge() > oldMessageAge+1){
        //if there is an unknown node between the previous node and this one create an empty node
        retString += "\\node (" + std::to_string(index) + ") at (" + std::to_string(x) + "," + std::to_string(y) + ") {};\n";
        std::pair<std::string, int> subRet = toTikzHelper(lowerX, upperX, y-yStep, yStep, oldMessageAge+1, index+1);
        retString += subRet.first;
        retInt = subRet.second;
        retString += "\\draw (" + std::to_string(index) + ") -- (" + std::to_string(index+1) + ");\n";
    }else{
        //draw this node
        retString += "\\node (" + std::to_string(index) + ") at (" + std::to_string(x) + "," + std::to_string(y) + ") {" + root.toTikz() +"};\n";

        //draw children
        //we need the return values of the children later, for the index calculation
        double xPerWidth = dist / maxWidth();
        std::vector<std::pair<std::string, int>> childRets;
        for(unsigned int i=0; i<children.size(); i++){
            upperX = lowerX + xPerWidth * children[i].maxWidth();
            if(i==0)
                childRets.push_back(children[i].toTikzHelper(lowerX, upperX, y-yStep, yStep, root.getMessageAge(), index+1));
            else
                childRets.push_back(children[i].toTikzHelper(lowerX, upperX, y-yStep, yStep, root.getMessageAge(), childRets[i-1].second+1));

            lowerX = upperX;
        }

        //add the strings in an additional loop, to have nicer formatting of the output
        for(auto ret : childRets)
            retString += ret.first;

        //draw lines to children
        if(childRets.size() >0){
            retString += "\\draw ";
            for(unsigned int i=0; i<childRets.size(); i++){
                if(i==0)
                    retString += "\n(" + std::to_string(index) + ") -- (" + std::to_string(index+1) + ")";
                else
                    retString += "\n(" + std::to_string(index) + ") -- (" + std::to_string(childRets[i-1].second+1) + ")";

            }
            retString += ";\n";
            //set return index to largest children index
            retInt = childRets[childRets.size()-1].second;
        }
    }

    return std::pair<std::string, int>(retString, retInt);
}


