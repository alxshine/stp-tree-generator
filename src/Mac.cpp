#include "../inc/Mac.hpp"

Mac::Mac():address("AABBAA"){ }

Mac::Mac(const Mac& other){
    address = other.address;
}

Mac::Mac(const u_char * const orig):address((const char*) orig, 6){
}

Mac::Mac(const std::string a):address(a){ }

Mac& Mac::operator=(const Mac& rhs){
    address = (rhs.address);
    return *this;
}

std::ostream& operator<<(std::ostream &out, const Mac& rhs){
    for(int i=0; i<5; i++)
        out << std::hex << std::setfill('0') << std::setw(2) <<  (int) rhs.address[i] << ":";
    return out << std::hex << std::setfill('0') << std::setw(2) << (int) rhs.address[5];
}

std::string Mac::getAddress () const{
    return address;
}

nlohmann::json Mac::toJson() const{
    nlohmann::json ret;
    ret["address"] = address;
    return ret;
}

int operator==(const Mac& lhs, const Mac& rhs){
    return lhs.address == rhs.address;
}

int operator!=(const Mac& lhs, const Mac& rhs){
    return lhs.address != rhs.address;
}

Mac Mac::fromJson(const nlohmann::json buildFrom){
    Mac ret;
    ret.address = buildFrom["address"];
    return ret;
}
