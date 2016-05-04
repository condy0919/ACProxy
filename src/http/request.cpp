#include "request.hpp"
#include <boost/lexical_cast.hpp>

namespace Http {

bool Request::isKeepAlive() const {
    auto iter = headers.find("Connection");
    if (iter == headers.end())
        return false;
    return iter->second == "keep-alive";
}

std::string Request::getHost() const {
    auto iter = headers.find("Host");
    if (iter == headers.end())
        return {};
    return iter->second;
}

std::size_t Request::getContentLength() const {
    auto iter = headers.find("Content-Length");
    if (iter == headers.end())
        return 0;
    return boost::lexical_cast<std::size_t>(iter->second);
}

std::ostream& operator<<(std::ostream& os, const Request& req) {
    os << req.method << " " << req.uri << " HTTP/" << req.http_version
       << "\r\n";

    for (auto&& header : req.headers) {
        os << header.first << ": " << header.second << "\r\n";
    }
    os << "\r\n";

    os << req.content.value_or("");
    return os;
}
}
