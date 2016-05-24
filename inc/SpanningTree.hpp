#pragma once
#include <vector>
#include <cassert>
#include <iostream>
#include <string>

#include "Bridge.hpp"
#include "../inc/json/json.h"
#include "../inc/json/json-forwards.h"

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
        Json::Value toJson() const;
        std::string toTikz(double lowerX, double upperX, int Y, int yStep, int oldMessageAge, int index) const;
        int maxWidth() const;
        int widthAtLevel(int levelsRemaining) const;
        Bridge getRoot() const;

        static SpanningTree fromJson(const Json::Value buildFrom);
        
        friend SpanningTree operator+(const SpanningTree& lhs, const SpanningTree& rhs);
        friend int operator==(const SpanningTree& lhs, const SpanningTree& rhs);
        friend int operator!=(const SpanningTree& lhs, const SpanningTree& rhs);
        friend std::ostream& operator<<(std::ostream &out, const SpanningTree& rhs);
        SpanningTree operator=(const SpanningTree& other);
};
