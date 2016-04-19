#include <iostream>

#include "../inc/json.hpp"
#include "../inc/SpanningTree.hpp"

using nlohmann::json;
using namespace std;

int main(){
    u_char buf1[] = "AABBAA";
    Mac m1(buf1);
    Bridge b1(m1, 3, 0);
    SpanningTree t1(b1);
    cout << t1.toJson() << endl;
}
