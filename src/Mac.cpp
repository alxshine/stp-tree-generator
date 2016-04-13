#include "../inc/Mac.hpp"

Mac::Mac(const Mac& other){
    std::memcpy(address, other.address, 6);
}

Mac::Mac(const u_char * const orig){
    std::memcpy(address, orig, 6);
}

Mac& Mac::operator=(const Mac& rhs){
    std::memcpy(address, rhs.address, 6);
    return *this;
}

std::ostream& operator<<(std::ostream &out, const Mac& rhs){
    for(int i=0; i<5; i++)
        out << std::hex << std::setfill('0') << std::setw(2) <<  (int) rhs.address[i] << ":";
    return out << std::hex << std::setfill('0') << std::setw(2) << (int) rhs.address[5];
}

std::string Mac::getAddressString () const{
    return std::string((const char*)address, 6);
}

int operator==(const Mac& lhs, const Mac& rhs){
    return lhs.getAddressString() == rhs.getAddressString();
}

int operator!=(const Mac& lhs, const Mac& rhs){
    return lhs.getAddressString() != rhs.getAddressString();
}
