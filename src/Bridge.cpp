#include "../inc/Bridge.hpp"

Bridge::Bridge(const Bridge& other):mac(other.mac), priority(other.priority), messageAge(other.messageAge)
{ }

Bridge::Bridge(Mac& m, const short p, const short mA):mac(m), priority(p), messageAge(mA){ }

std::ostream& operator<<(std::ostream &out, const Bridge& rhs){
    return out << rhs.priority << " / " << rhs.mac << ", Message age: " << rhs.messageAge;
}

const Mac& Bridge::getMac() const
{
    return mac;
}

const short Bridge::getMessageAge() const 
{
    return messageAge;
}

const short Bridge::getPriority() const
{
    return priority;
}

nlohmann::json Bridge::toJson() const{
    nlohmann::json ret;
    ret["mac"] = mac.toJson();
    ret["priority"] = priority;
    ret["messageAge"] = messageAge;
    return ret;
}

int operator==(const Bridge& lhs, const Bridge& rhs){
    return lhs.priority == rhs.priority && lhs.messageAge == rhs.messageAge && lhs.mac == rhs.mac;
}

int operator!=(const Bridge& lhs, const Bridge& rhs){
    return lhs.priority != rhs.priority || lhs.messageAge != rhs.messageAge || lhs.mac != rhs.mac;
}

Bridge& Bridge::operator=(const Bridge& other){
    mac = other.mac;
    messageAge = other.messageAge;
    priority = other.priority;
    return *this;
}
