#pragma once
#include <vector>
#include <cassert>
#include <iostream>

#include "Bridge.hpp"
#include "../inc/json.hpp"

class SpanningTree{
    private:
        Bridge root;
        std::vector<SpanningTree> children;

    public:
        SpanningTree();
        SpanningTree(const Bridge& r);
        SpanningTree(const Bridge& r, std::vector<SpanningTree> c);
        SpanningTree(const SpanningTree& other);
        ~SpanningTree();
        void addChild(const SpanningTree& child);
        int addSubTree(const SpanningTree& other);
        int containsRoot(const SpanningTree& tree) const;
        nlohmann::json toJson() const;

        static SpanningTree fromJson(const nlohmann::json buildFrom);
        
        friend SpanningTree operator+(const SpanningTree& lhs, const SpanningTree& rhs);
        friend int operator==(const SpanningTree& lhs, const SpanningTree& rhs);
        friend int operator!=(const SpanningTree& lhs, const SpanningTree& rhs);
        friend std::ostream& operator<<(std::ostream &out, const SpanningTree& rhs);
        SpanningTree operator=(const SpanningTree& other);
};
