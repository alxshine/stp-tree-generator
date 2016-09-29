        //draw edges to children
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
