#include "request.hpp"
#include <boost/lexical_cast.hpp>

namespace Http {
void Request::setKeepAlive() {
    headers["Connection"] = "keep-alive";
}

void Request::setNoKeepAlive() {
    headers["Connection"] = "close";
}

bool Request::isKeepAlive() const {
    // TODO consider HTTP/1.0 and HTTP/1.1 
    auto iter = headers.find("Connection");
    if (iter == headers.end())
        return false;
    return iter->second == "keep-alive";
}

const std::string Request::getHost() const {
    auto iter = headers.find("Host");
    if (iter == headers.end())
        return {};
    std::string::size_type pos = iter->second.find_first_of(":");
    if (pos != std::string::npos) {
        return iter->second.substr(0, pos);
    }
    return iter->second;
}

const int Request::getPort() const {
    if (method == "CONNECT") { // no host header, parse from uri
        std::string::size_type pos = uri.find_first_of(":");
        if (pos != std::string::npos) {
            return boost::lexical_cast<int>(uri.substr(pos + 1));
        }
    } else {
        std::string host = getHost();
        std::string::size_type pos = host.find_first_of(":");
        if (pos != std::string::npos) {
            return boost::lexical_cast<int>(host.substr(pos + 1));
        }
    }
    return 80;
}

std::size_t Request::getContentLength() const {
    auto iter = headers.find("Content-Length");
    if (iter == headers.end())
        return 0;
    return boost::lexical_cast<std::size_t>(iter->second);
}

void Request::rewrite() {
    std::string uri_ = uri;
    std::string host = getHost();
    // TODO
}

std::string Request::toBuffer() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}

bool Request::isConnectMethod() const {
    return method == "CONNECT";
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
