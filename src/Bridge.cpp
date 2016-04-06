#include "../inc/Bridge.hpp"

Bridge::Bridge(const Bridge& other):mac(other.mac), priority(other.priority), messageAge(other.messageAge)
{ }

Bridge::Bridge(const Mac& m, const short p, const short mA):mac(m),priority(p), messageAge(mA){ }

std::ostream& operator<<(std::ostream &out, const Bridge& rhs){
    return out << rhs.priority << " / " << rhs.mac << ", Message age: " << rhs.messageAge;
}

const Mac& Bridge::getMac()
{
    return mac;
}

const short Bridge::getMessageAge()
{
    return messageAge;
}

const short Bridge::getPriority()
{
    return priority;
}

