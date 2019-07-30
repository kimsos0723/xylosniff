#include "../includes/core/mac.h"
#include <algorithm>

using namespace core;

HWAddr::HWAddr(const string& mac_str)
{
    std::smatch m;
    regex octet("([0-9a-fA-f]{2})[:,-]?([0-9a-fA-f]{2})[:,-]?([0-9a-fA-f]{2})[:,-]?([0-9a-fA-f]{2})[:,-]?([0-9a-fA-f]{2})[:,-]?([0-9a-fA-f]{2})");
    if (std::regex_match(mac_str, m, octet)) {
        for (int i = 1; i < MACSIZE + 1; i++) {
            mac[i - 1] = stoi(m[i].str(), nullptr, 16);
        }
    } else {
        throw std::invalid_argument("Worng MAC Address Syntax");
    }
}

HWAddr::operator string() const
{
    stringstream ss;
    for(auto& i : this->mac) {
        ss << std::hex << int(i);        
    }
    return ss.str();
}
HWAddr::operator uint64_t() const
{
}

ostream& operator<<(ostream& o, const mac_t& m)
{
    return o;
}