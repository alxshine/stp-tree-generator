#include <iostream>

#include "../inc/json/json.h"
#include "../inc/json/json-forwards.h"

using namespace std;

int main(void){
    Json::Value v1;
    v1["test"] = "aoeu";
    Json::FastWriter writer;

    cout << writer.write(v1) << endl;
}
