#ifndef HTTP_HEADER_HPP_
#define HTTP_HEADER_HPP_

#include <string>

namespace Http {
struct Header {
    std::string name;
    std::string value;
};
}

#endif
