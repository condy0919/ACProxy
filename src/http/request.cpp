#include "request.hpp"
#include <boost/lexical_cast.hpp>
#include <algorithm>

namespace Http {
void Request::setKeepAlive(bool on) {
    setHeader("Connection", on ? "keep-alive" : "close");
}

bool Request::isKeepAlive() const {
    // TODO consider HTTP/1.0 and HTTP/1.1
    return getHeader("Connection") == "keep-alive";
}

bool Request::hasContentBody() const {
    return method == "POST" || method == "PUT" || method == "CONNECT" ||
           method == "PATCH";
}

bool Request::hasResponseBody() const {
    return method != "HEAD";
}

const std::string Request::getHost() const {
    std::string host = getHeader("Host");
    std::string::size_type pos = host.find_first_of(":");
    return host.substr(0, pos);
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
    std::string len = getHeader("Content-Length");
    if (len.empty())
        return 0;
    return boost::lexical_cast<std::size_t>(len);
}

// case 1:
// GET http://baidu.com/index.html HTTP/1.1 ->
// GET /index.html HTTP/1.1
//
// case 2:
// GET http://3g.sina.com.cn HTTP/1.1 ->
// GET / HTTP/1.1
void Request::rewrite() {
    const static std::string prefix("http://");
    auto iter =
        std::search(uri.begin(), uri.end(), prefix.begin(), prefix.end());

    if (iter == uri.end()) {
        return;
    }
    auto slash_pos = std::find(iter + 7, uri.end(), '/');
    if (slash_pos != uri.end()) {
        uri = std::string(slash_pos, uri.end());
    } else {
        uri = "/";
    }
}

std::string Request::toBuffer() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}

bool Request::isConnectMethod() const {
    return method == "CONNECT";
}

void Request::setHeader(std::string name, std::string value) {
    auto iter = std::find_if(headers.begin(), headers.end(),
                             [&](auto&& pair_) { return pair_.first == name; });
    if (iter == headers.end()) {
        headers.emplace_back(name, value);
    } else {
        iter->second = value;
    }
}

const std::string Request::getHeader(std::string name) const {
    auto iter = std::find_if(headers.begin(), headers.end(),
                            [&](auto&& pair_) { return pair_.first == name; });
    if (iter == headers.end()) {
        return {};
    } else {
        return iter->second;
    }
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
