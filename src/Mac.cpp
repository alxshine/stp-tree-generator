#include "../inc/Mac.hpp"

Mac::Mac(const Mac& other)
{
    std::memcpy(address, other.address, 6);
}

Mac::Mac(const char * const orig)
{
    std::memcpy(address, orig, 6);
}

Mac& Mac::operator=(const Mac& rhs)
{
    std::memcpy(address, rhs.address, 6);
    return *this;
}

