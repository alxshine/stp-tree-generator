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

}

void SpanningTree::addChild(SpanningTree& child){
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
