#pragma once
#include <vector>

#include "Bridge.hpp"

class SpanningTree{
    private:
        Bridge root;
        std::vector<SpanningTree> children;

    public:
        SpanningTree(const Bridge& r);
        SpanningTree(const Bridge& r, std::vector<SpanningTree> c);
        ~SpanningTree();
        void addChild(SpanningTree& child);
        friend SpanningTree operator+(SpanningTree& lhs, SpanningTree& rhs);
};
