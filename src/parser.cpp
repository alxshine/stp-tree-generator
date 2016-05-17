#include <iostream>
#include <fstream>

#include "../inc/SpanningTree.hpp"

using namespace std;

int main(int argc, char ** args){
    string test = "{\"children\":[{\"children\":null,\"root\":{\"mac\":{\"address\":\"00:26:F1:A7:CA:00\"},\"messageAge\":1,\"priority\":128}}],\"root\":{\"mac\":{\"address\":\"84:34:97:C8:A1:00\"},\"messageAge\":0,\"priority\":64}}";
    Json::Reader reader;
    Json::Value jsonValue;
    reader.parse(test.c_str(), jsonValue);
    SpanningTree testTree = SpanningTree::fromJson(jsonValue);

    ofstream outputFile("test.tikz", std::ios::out);
    if(!outputFile.is_open()){
        cout << "could not open test.tikz";
        return -1;
    }

    outputFile << "\\begin{tikzpicture}[transform shape]" << endl;
    outputFile << testTree.toTikz(0, 100, 0, 20, 0) << endl;

    outputFile << "\\end{tikzpicture}";
    outputFile.close();
}
