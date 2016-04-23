#ifndef ACPROXY_IP_HPP_
#define ACPROXY_IP_HPP_

#include <memory>
#include <utility>
#include <string>
#include <cstring>

namespace ACProxy {

class IP {
public:
    IP(std::string s) noexcept {
        assign(s);
    }

    IP(const char* ip) noexcept {
        assign(ip, std::strlen(ip));
    }

private:
    std::shared_ptr<const char> ptr;
};
}

#endif
