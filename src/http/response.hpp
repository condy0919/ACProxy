#ifndef ACPROXY_HTTP_RESPONSE_HPP_
#define ACPROXY_HTTP_RESPONSE_HPP_

#include <string>
#include <map>

namespace Http {
struct Response {
    int http_version_major;
    int http_version_minor;
    int http_status_code;
    std::string http_status_msg;

    std::map<std::string, std::string> headers;
    std::string content;
};
}

#endif
