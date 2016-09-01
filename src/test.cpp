#include<iostream>
#include<fstream>
#include<string>

#include "../inc/SpanningTree.hpp"
#include "../inc/json/json.h"

using namespace std;

int main(int argc, char **argv){
    if(argc < 2){
        cerr << "no filename provided" << endl;
        return -1;
    }

    fstream ifstream = fstream(argv[1]);
    string line, file;
    if(ifstream.is_open()){
        while(getline(ifstream, line))
            file += line + "\n";
    }else{
        cerr << "could not open file" << endl;
        return -1;
    }

    Json::Value json;
    Json::Reader reader;
    reader.parse(file, json);
    SpanningTree tree = SpanningTree::fromJson(json);
    cout << tree.toTikz(0,14,20,2,0,0) << endl;
}
