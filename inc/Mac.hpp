#pragma once
#include <cstring>
#include <iostream>
#include <iomanip>

#include "../inc/json/json.h"
#include "../inc/json/json-forwards.h"

class Mac{
    private:
        std::string address;

    public:
        Mac();
        Mac(const u_char * const orig);
        Mac(const std::string a);
        Mac(const Mac& other);
        Mac& operator=(const Mac& rhs);
        std::string getAddress() const;
        Json::Value toJson() const;

        static Mac fromJson(const Json::Value buildFrom);
        
        friend std::ostream& operator<<(std::ostream &out, const Mac& rhs);
        friend int operator==(const Mac& lhs, const Mac& rhs);       
        friend int operator!=(const Mac& lhs, const Mac& rhs);       
};
