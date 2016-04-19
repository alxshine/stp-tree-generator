#include <iostream>

#include "../inc/json.hpp"
#include "../inc/SpanningTree.hpp"

using namespace nlohmann;
using namespace std;

int main(void){
    Mac m1("AABBAA");
    Bridge b1(m1, 0, 0);
    Mac m2("AAAAAA");
    Mac m3("BBBBBB");
    Bridge b2(m2, 0, 1);
    Bridge b3(m3, 0, 1);
    SpanningTree s1(b1);
    s1.addChild(SpanningTree(b2));
    s1.addChild(SpanningTree(b3));
    json j1 = s1.toJson();
    cout << "generated json: " << j1 << endl;
    cout << "generated tree: " << SpanningTree::fromJson(j1) << endl;
}

