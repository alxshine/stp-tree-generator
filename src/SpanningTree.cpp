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
    Bridge newBridge = Bridge::fromJson(buildFrom["root"]);
    ret.root = newBridge;
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
