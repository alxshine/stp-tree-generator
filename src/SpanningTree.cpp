#include "../inc/SpanningTree.hpp"

SpanningTree::SpanningTree(){}

SpanningTree::SpanningTree(const Bridge& r):root(r){
}

SpanningTree::SpanningTree(const Bridge& r, std::vector<SpanningTree> c): root(r), children(c){
}

SpanningTree::SpanningTree(const SpanningTree& other):root(other.root), children(other.children){
}

SpanningTree::~SpanningTree(){
}

SpanningTree operator+(const SpanningTree& lhs, const SpanningTree& rhs){
    if(lhs == rhs){
        return SpanningTree(lhs);
    }

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

void SpanningTree::addChild(const SpanningTree& child){
    children.push_back(SpanningTree(child));
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
        for(const SpanningTree& childR : rhs.children){
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

int SpanningTree::addSubTree(const SpanningTree& other)
{
    if(!containsRoot(other))
        return 0;

    if(root == other.root){
        //combine all similar children
        for(SpanningTree leftChild : children){
            for(SpanningTree rightChild : other.children){
                if(leftChild.root == rightChild.root){
                    leftChild.addSubTree(rightChild);
                    break;
                }
            }
        }

        //add all missing right children to the tree
        for(SpanningTree rightChild : other.children){
            int contained = 0;
            for(SpanningTree leftChild : children){
                if(leftChild.root == rightChild.root){
                    contained = 1;
                    break;
                }
            }
            if(!contained)
                addChild(other);
        }
    }

    return 1;
}

Json::Value SpanningTree::toJson() const {
    Json::Value ret, c;
    ret["root"] = root.toJson();

    for(SpanningTree tree : children)
        c.append(tree.toJson());

    ret["children"] = c;
    return ret;
}

std::string SpanningTree::toTikz(double lowerX, double upperX, int y, int yStep, int oldMessageAge, int index) const {
    std::string ret;

    double dist = upperX - lowerX;
    double x = lowerX + dist/2;

    if(root.getMessageAge() > oldMessageAge+1){
        //if there is an unknown node between the previous node and this one create an empty node
        ret += "\\node (" + std::to_string(index) + ") at (" + std::to_string(x) + "," + std::to_string(y) + ") {};\n";
        ret += toTikz(lowerX, upperX, y+yStep, yStep, oldMessageAge+1, index+1);
        ret += "\\draw (" + std::to_string(index) + ") -- (" + std::to_string(index+1) + ");\n";
    }else{
        //draw this node
        ret += "\\node (" + std::to_string(index) + ") at (" + std::to_string(x) + "," + std::to_string(y) + ") {" + root.toTikz() +"};\n";

        //draw children
        double xPerWidth = dist / maxWidth();
        //the indices of the children have to be calculated
        //the required indices for a subtree equals its size
        int childIndex = index+1;
        for(SpanningTree s : children){
            upperX = lowerX + xPerWidth * s.maxWidth();
            ret += s.toTikz(lowerX, upperX, y-yStep, yStep, oldMessageAge+1, childIndex);
            lowerX = upperX;
            childIndex += s.size();
        }

        //draw lines to children
        if(children.size() >0){
            ret += "\\draw ";
            //same indices calculation
            childIndex = index+1;
            for(SpanningTree child : children){
                ret += "\n(" + std::to_string(index) + ") -- (" + std::to_string(childIndex) + ")";
                childIndex += child.size();
            }
            ret += ";\n";
        }
    }

    return ret;
}

int SpanningTree::maxWidth() const{
    int max = 1;
    for(int level = 0, currentWidth; (currentWidth = widthAtLevel(level)) > 0; level++)
        max = max > currentWidth ? max : currentWidth;

    return max;
}

int SpanningTree::widthAtLevel(int levelsRemaining) const{
    if(levelsRemaining <= 0)
        return 1;
    if(levelsRemaining == 1)
        return children.size();

    int ret = 0;
    for(SpanningTree s : children)
        ret += s.widthAtLevel(levelsRemaining-1);
    return ret;
}

int SpanningTree::size() const{
    int size = 1;
    for(SpanningTree child : children)
        size += child.size();
    
    return size;
}

int SpanningTree::containsRoot(const SpanningTree& tree) const{
    if(root==tree.root)
        return 1;

    for(SpanningTree st : children)
        if(st.containsRoot(tree))
                return 1;

    return 0;
}

Bridge SpanningTree::getRoot() const{
    return Bridge(root);
}

SpanningTree SpanningTree::fromJson(const Json::Value buildFrom){
    SpanningTree ret;
    ret.root = Bridge::fromJson(buildFrom["root"]);
    auto childrenJson = buildFrom["children"];
    for(auto cj : childrenJson)
        ret.children.push_back(SpanningTree::fromJson(cj));

    return ret;
}

SpanningTree SpanningTree::operator=(const SpanningTree& other){
    root = Bridge(other.root);
    children = other.children;
    return *this;
}
