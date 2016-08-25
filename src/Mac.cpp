#include "../inc/Mac.hpp"

Mac::Mac():address("AABBAA"){ }

Mac::Mac(const Mac& other){
    address = other.address;
}

Mac::Mac(const u_char * const orig){
    char buffer[17];
    sprintf(buffer,"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", orig[0], orig[1], orig[2], orig[3], orig[4], orig[5]);
    address = std::string(buffer, 17);
}

Mac::Mac(const std::string a):address(a){ }

Mac& Mac::operator=(const Mac& rhs){
    address = (rhs.address);
    return *this;
}

std::string Mac::getAddress () const{
    return address;
}

Json::Value Mac::toJson() const{
    Json::Value ret;
    ret["address"] = address;
    return ret;
}

int operator==(const Mac& lhs, const Mac& rhs){
    return lhs.address == rhs.address;
}

int operator!=(const Mac& lhs, const Mac& rhs){
    return lhs.address != rhs.address;
}

Mac Mac::fromJson(const Json::Value buildFrom){
    Mac ret;
    ret.address = buildFrom["address"].asString();
    return ret;
}
