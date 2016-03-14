#pragma once
#include <cstring>
#include<iostream>
class Mac{
    private:
        unsigned char address[6];

    public:
        Mac(const u_char * const orig);
        Mac(const Mac& other);
        Mac& operator=(const Mac& rhs);
        friend std::ostream& operator<<(std::ostream &out, const Mac& rhs);
};
