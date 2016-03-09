#include "../inc/Mac.hpp"

Mac::Mac(const Mac& other){
    std::memcpy(address, other.address, 6);
}

Mac::Mac(const char * const orig){
    std::memcpy(address, orig, 6);
}

Mac& Mac::operator=(const Mac& rhs){
    std::memcpy(address, rhs.address, 6);
    return *this;
}

std::ostream& operator<<(std::ostream &out, const Mac& rhs){
    out << std::hex;
    for(int i=0; i<5; i++)
        out << rhs.address[i] << ":";
    return out << rhs.address[5];
}

