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
