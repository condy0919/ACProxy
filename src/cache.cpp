#include "cache.hpp"

namespace ACProxy {

std::string keygen(std::string host, int port, std::string uri,
                   std::string method) {
    std::string key = host + std::to_string(port) + uri + method;
    return key;
}
}
