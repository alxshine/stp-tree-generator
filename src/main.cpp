#include <iostream>

#include "../inc/SpanningTree.hpp"
#include "../inc/Bridge.hpp"

using namespace std;

int main(int argc, char **args){
    //test layout is:
    //root  - second - fourth
    //      - third
    
    u_char rootAddress[] = {0, 0x10, 0x95, 0xde, 0xad, 0x07};
    Mac rootMac = Mac(rootAddress);
    Bridge root = Bridge(rootMac, 0, 0);
 
    u_char secondAddress[] = {0x70, 0x70, 0x95, 0xde, 0xad, 0x07};
    Mac secondMac = Mac(secondAddress);
    Bridge second(secondMac, 0, 1);
    
    u_char thirdAddress[] = {0x71, 0x10, 0x95, 0xdA, 0xad, 0x07};
    Mac thirdMac = Mac(thirdAddress);
    Bridge third(thirdMac, 0, 1);
    
    u_char fourthAddress[] = {0x72, 0x20, 0x95, 0xdB, 0xad, 0x07};
    Mac fourthMac = Mac(fourthAddress);
    Bridge fourth(fourthMac, 0, 2);

    SpanningTree treeA = SpanningTree(root);
    SpanningTree childA(second);
    treeA.addChild(childA);
    SpanningTree treeB = SpanningTree(root);
    SpanningTree childB(third);
    treeB.addChild(childB);
    childA.addChild(SpanningTree(fourth));
    treeB.addChild(childA);
    SpanningTree combined = treeA+treeB;

    cout << "B:" << endl;
    cout << treeB << endl;
    cout << "A:" << endl;
    cout << treeA << endl;
    cout << "combined: " << endl;
    cout << combined << endl;
//    SpanningTree treeC = SpanningTree(root);
//    SpanningTree temp = SpanningTree(second);
//    temp.addChild(SpanningTree(fourth));
//    treeC.addChild(temp);
//    cout << "root: " << root << ", second: " << second << ", third: " << third << ", fourth: " << fourth << endl;
//    cout << "A" << endl;
//    cout << treeA;
//    cout << "B" << endl;
//    cout << treeB;
//    cout << "C" << endl;
//    cout << treeC;
//    cout << "A + B" << endl;
//    cout << combined;
}
