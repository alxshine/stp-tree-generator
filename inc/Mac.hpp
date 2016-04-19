#pragma once
#include <cstring>
#include <iostream>
#include <iomanip>

#include "../inc/json.hpp"

class Mac{
    private:
        unsigned char address[6];

    public:
        Mac(const u_char * const orig);
        Mac(const Mac& other);
        Mac& operator=(const Mac& rhs);
        std::string getAddressString() const;
        nlohmann::json toJson() const;

        
        friend std::ostream& operator<<(std::ostream &out, const Mac& rhs);
        friend int operator==(const Mac& lhs, const Mac& rhs);       
        friend int operator!=(const Mac& lhs, const Mac& rhs);       
};
