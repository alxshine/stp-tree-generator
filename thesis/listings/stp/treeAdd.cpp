SpanningTree operator+(const SpanningTree& lhs, const SpanningTree& rhs){
    if(lhs == rhs){
        return SpanningTree(lhs);
    }

    SpanningTree ret = SpanningTree(lhs.root);

    //add all similar children
    for(SpanningTree leftChild : lhs.children){
        for(SpanningTree rightChild : rhs.children){
            //if they have the same root, add the combination
            if(leftChild.root == rightChild.root){
                //should we make an incorrect assumption, set the message age to the higher one.
                //this should not be needed, as this should be filtered in the server.
                SpanningTree newTree = leftChild + rightChild;
                newTree.root.setMessageAge(std::max(leftChild.root.getMessageAge(), rightChild.root.getMessageAge()));
                ret.addChild(newTree);
                break;
            }

            //if rightChild is contained in left (additions "downstream"), add it there
            if(leftChild.containsRoot(rightChild)){
                SpanningTree combined = SpanningTree(leftChild);
                leftChild.addSubTree(rightChild);
                ret.addChild(combined);
            }
            
            //vice versa
            if(rightChild.containsRoot(leftChild)){
                SpanningTree combined = SpanningTree(rightChild);
                leftChild.addSubTree(leftChild);
                ret.addChild(combined);
            }
        }
    }

    //add all remaining children of lhs
    for(SpanningTree leftChild : lhs.children){
        int contained = 0;
        for(SpanningTree added : ret.children){
            if(added.root == leftChild.root){
                contained = 1;
                break;
            }
        }
        if(!contained){
            ret.addChild(leftChild);
        }
    }
    //add all remaining children of rhs
    for(SpanningTree rightChild : rhs.children){
        int contained = 0;
        for(SpanningTree added : ret.children){
            if(added.root == rightChild.root){
                contained = 1;
                break;
            }
        }
        if(!contained){
            ret.addChild(rightChild);
        }
    }

    return ret;
}
