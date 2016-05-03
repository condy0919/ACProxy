#ifndef ACPROXY_HTTP_RESPONSE_HPP_
#define ACPROXY_HTTP_RESPONSE_HPP_

#include "header.hpp"
#include <string>
#include <vector>

namespace Http {
struct Response {
    int http_version_major;
    int http_version_minor;
    int http_status_code;
    std::string http_status_msg;

    std::vector<Header> headers;
    std::string content;
};
}

#endif
