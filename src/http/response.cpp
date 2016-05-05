#include "response.hpp"
#include <boost/lexical_cast.hpp>

namespace Http {
bool Response::isKeepAlive() const {
    // TODO consider HTTP/1.0 and HTTP/1.1
    auto iter = headers.find("Connection");
    if (iter == headers.end())
        return false;
    return iter->second == "keep-alive";
}

const std::string Response::getContentType() const {
    auto iter = headers.find("Content-Type");
    if (iter == headers.end())
        return {};
    return iter->second;
}

const std::size_t Response::getContentLength() const {
    auto iter = headers.find("Content-Length");
    if (iter == headers.end())
        return 0;
    return boost::lexical_cast<std::size_t>(iter->second);
}

const std::string Response::getContent() const {
    return content.value_or("");
}

std::ostream& operator<<(std::ostream& os, const Response& resp) {
    os << "HTTP/" << resp.http_version << " " << resp.status_code << " "
       << resp.description << "\r\n";
    for (auto&& header : resp.headers) {
        os << header.first << ": " << header.second << "\r\n";
    }
    os << "\r\n";
    os << resp.getContent();
    return os;
}
}
