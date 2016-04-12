#include "../inc/SpanningTree.hpp"

SpanningTree::SpanningTree(const Bridge& r):root(r){
}

SpanningTree::SpanningTree(const Bridge& r, std::vector<SpanningTree> c): root(r), children(c){
}

SpanningTree::~SpanningTree(){
}

SpanningTree operator+(const SpanningTree& lhs, const SpanningTree& rhs){
    if(lhs == rhs)
        return SpanningTree(lhs);

    assert(lhs.root == rhs.root);
    SpanningTree ret = SpanningTree(lhs.root);

    //add all similar children
    for(SpanningTree leftChild : lhs.children){
        for(SpanningTree rightChild : rhs.children){
            //if they have the same root, add the combination
            if(leftChild.root == rightChild.root){
                ret.addChild(leftChild + rightChild);
                break;
            }

            //if rightChild is contained in left (additions "downstream"), add it there
            if(leftChild.contains(rightChild)){
                SpanningTree combined = SpanningTree(leftChild);
                leftChild.addSubTree(rightChild);
                ret.addChild(combined);
            }
            
            //vice versa
            if(rightChild.contains(leftChild)){
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
        if(!contained)
            ret.addChild(leftChild);
    }
    //add all remaining children of rhs
    for(SpanningTree rightChild : lhs.children){
        int contained = 0;
        for(SpanningTree added : ret.children){
            if(added.root == rightChild.root){
                contained = 1;
                break;
            }
        }
        if(!contained)
            ret.addChild(rightChild);
    }

    return ret;
}

void SpanningTree::addChild(const SpanningTree& child){
    children.push_back(child);
}

int operator==(const SpanningTree& lhs, const SpanningTree& rhs){
    if(lhs.root != rhs.root)
        return 0;

    //check if both have the same amount of children (if not, they are not the same)
    if(lhs.children.size() != rhs.children.size())
        return 0;

    //check if all children of lhs are contained in the children of rhs
    //if this holds, all children of rhs are going to be contained in lhs
    //(because they have the same amount of children)
    for(const SpanningTree& childL : lhs.children){
        int contained = 0;
        for(const SpanningTree& childR : lhs.children){
            if(childL == childR){
                contained = 1;
                break;
            }
        }
        if(!contained)
            return 0;
    }

    return 1;
}

int operator!=(const SpanningTree& lhs, const SpanningTree& rhs){
    return !(lhs==rhs);
}
