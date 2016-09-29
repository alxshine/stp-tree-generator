    if(root.getMessageAge() > oldMessageAge+1){
        //if there is an unknown node between the previous node and this one create an empty node
        retString += "\\node (" + std::to_string(index) + ") at (" + std::to_string(x) + "," + std::to_string(y) + ") {};\n";
        std::pair<std::string, int> subRet = toTikzHelper(lowerX, upperX, y-yStep, yStep, oldMessageAge+1, index+1);
        retString += subRet.first;
        retInt = subRet.second;
        retString += "\\draw (" + std::to_string(index) + ") -- (" + std::to_string(index+1) + ");\n";

