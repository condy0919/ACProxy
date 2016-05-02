#ifndef HTTP_REQUEST_HPP_
#define HTTP_REQUEST_HPP_

#include "header.hpp"
#include <string>
#include <vector>
#include <ostream>

namespace Http {
struct Request {
    std::string method;
    std::string uri;
    int http_version_major;
    int http_version_minor;
    std::vector<Header> headers;
};

std::ostream& operator<<(std::ostream&, Request);
}

#endif
