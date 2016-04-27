#ifndef ACPROXY_HTTP_RESPONSE_HPP_
#define ACPROXY_HTTP_RESPONSE_HPP_

#include "header.hpp"
#include <string>
#include <vector>

namespace Http {
struct Response {
    enum Status {
        ok = 200,
        created = 201,
        accepted = 202,
        no_content = 204,
        multiple_choices = 300,
        moved_permanently = 301,
        moved_temporarily = 302,
        not_modified = 304,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503
    } status;

    std::vector<Header> headers;
    std::string content;
    std::vector<char> to_buffers();
};
}

#endif
