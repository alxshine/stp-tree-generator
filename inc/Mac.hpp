#pragma once
#include <cstring>
#include <iostream>
#include <iomanip>

#include "../inc/json.hpp"

class Mac{
    private:
        std::string address;

    public:
        Mac();
        Mac(const u_char * const orig);
        Mac(const std::__cxx11::string a);
        Mac(const Mac& other);
        Mac& operator=(const Mac& rhs);
        std::string getAddress() const;
        nlohmann::json toJson() const;

        static Mac fromJson(const nlohmann::json buildFrom);
        
        friend std::ostream& operator<<(std::ostream &out, const Mac& rhs);
        friend int operator==(const Mac& lhs, const Mac& rhs);       
        friend int operator!=(const Mac& lhs, const Mac& rhs);       
};
