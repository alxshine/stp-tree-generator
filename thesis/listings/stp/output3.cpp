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


