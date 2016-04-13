#pragma once
#include <iostream>

#include "Mac.hpp"

class Bridge{
    private:
        Mac& mac;
        short priority;
        short messageAge;
        
    public:
        Bridge(const Mac& m, const short p, const short mA);
        Bridge(const Bridge& other);
        const Mac& getMac() const;
        const short getPriority() const ;
        const short getMessageAge() const;

        friend std::ostream& operator<<(std::ostream &out, const Bridge& rhs);
        friend int operator==(const Bridge& lhs, const Bridge& rhs);
        friend int operator!=(const Bridge& lhs, const Bridge& rhs);
        Bridge operator=(const Bridge& other);
};
