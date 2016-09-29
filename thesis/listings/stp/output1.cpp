std::string SpanningTree::toTikz(double lowerX, double upperX, int y, int yStep, int oldMessageAge, int index) const {
    return toTikzHelper(lowerX, upperX, y, yStep, oldMessageAge, index).first;
}

std::pair<std::string, int> SpanningTree::toTikzHelper(double lowerX, double upperX, int y, int yStep, int oldMessageAge, int index) const {
    //this helper is needed because the previously used child index calculation was faulty if the algorithm was adding empty nodes
    std::string retString;
    int retInt = index;

    double dist = upperX - lowerX;
    double x = lowerX + dist/2;
