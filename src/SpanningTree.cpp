#include "../inc/SpanningTree.hpp"

SpanningTree::SpanningTree(const Bridge& r):root(r)
{
}

SpanningTree::SpanningTree(const Bridge& r, std::vector<SpanningTree> c): root(r), children(c)
{
}

SpanningTree::~SpanningTree()
{
}

SpanningTree operator+(SpanningTree& lhs, SpanningTree& rhs)
{
    return /* something */;
}

void SpanningTree::addChild(SpanningTree& child)
{
    children.push_back(child);
}

