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
    std::cout << "checking for similar children" << std::endl;
    for(SpanningTree leftChild : lhs.children){
        std::cout << "leftChild: " << leftChild << std::endl;
        for(SpanningTree rightChild : rhs.children){
            std::cout << "rightChild: " << rightChild << std::endl;
            //if they have the same root, add the combination
            if(leftChild.root == rightChild.root){
                std::cout << "same root" << std::endl;
                ret.addChild(leftChild + rightChild);
                break;
            }

            //if rightChild is contained in left (additions "downstream"), add it there
            if(leftChild.containsRoot(rightChild)){
                std::cout << "right contained in left" << std::endl;
                SpanningTree combined = SpanningTree(leftChild);
                leftChild.addSubTree(rightChild);
                ret.addChild(combined);
            }
            
            //vice versa
            if(rightChild.containsRoot(leftChild)){
                std::cout << "left contained in right" << std::endl;
                SpanningTree combined = SpanningTree(rightChild);
                leftChild.addSubTree(leftChild);
                ret.addChild(combined);
            }
        }
    }

    //add all remaining children of lhs
    std::cout << "adding remaining children of lhs" << std::endl;
    for(SpanningTree leftChild : lhs.children){
        int contained = 0;
        for(SpanningTree added : ret.children){
            if(added.root == leftChild.root){
                contained = 1;
                break;
            }
        }
        if(!contained){
            std::cout << "left child not contained, adding" << std::endl;
            ret.addChild(leftChild);
        }
    }
    //add all remaining children of rhs
    std::cout << "adding remaining children of rhs" << std::endl;
    for(SpanningTree rightChild : rhs.children){
        int contained = 0;
        for(SpanningTree added : ret.children){
            if(added.root == rightChild.root){
                contained = 1;
                break;
            }
        }
        if(!contained){
            std::cout << "right child not contained, adding" << std::endl;
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

std::string SpanningTree::toTikz(double lowerX, double upperX, int y, int yStep, int index) const {
    std::string ret;

    double dist = upperX - lowerX;
    double x = lowerX + dist/2;
 
    ret += "\\node (" + std::to_string(index) + ") at (" + std::to_string(x) + ":" + std::to_string(y) + ") {" + std::string(root) +"}\n";

    double xPerWidth = dist / maxWidth();
    int initialIndex = index;
    for(SpanningTree s : children){
        upperX = lowerX + xPerWidth * s.maxWidth();
        ret += s.toTikz(lowerX, upperX, y+yStep, yStep, ++index);
        lowerX = upperX;
    }

    ret += "\\draw ";
    for(int i = initialIndex+1; i <= index; i++)
        ret += "(" + std::to_string(initialIndex) + ") -- (" + std::to_string(i) + ")\n";

    ret += ";\n";
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

std::ostream& operator<<(std::ostream &out, const SpanningTree& rhs){
    out << rhs.root << std::endl;
    out << "children: " << std::endl;
    for(SpanningTree child : rhs.children)
        out << "-" << child;
    out << std::endl;

    return out;
}

SpanningTree SpanningTree::operator=(const SpanningTree& other){
    root = Bridge(other.root);
    children = other.children;
    return *this;
}
