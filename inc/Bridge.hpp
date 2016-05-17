#pragma once
#include <iostream>

#include "Mac.hpp"
#include "../inc/json/json.h"
#include "../inc/json/json-forwards.h"

class Bridge{
    private:
        Mac mac;
        short priority;
        short messageAge;
        
    public:
        Bridge();
        Bridge(Mac& m, const short p, const short mA);
        Bridge(const Bridge& other);
        const Mac& getMac() const;
        const short getPriority() const ;
        const short getMessageAge() const;
        Json::Value toJson() const;

        static Bridge fromJson(const Json::Value buildFrom);

        operator std::string() const;
        friend std::ostream& operator<<(std::ostream &out, const Bridge& rhs);
        friend int operator==(const Bridge& lhs, const Bridge& rhs);
        friend int operator!=(const Bridge& lhs, const Bridge& rhs);
        Bridge& operator=(const Bridge& other);
};
