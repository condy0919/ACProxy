#include "response.hpp"
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <sstream>

namespace Http {
bool Response::isKeepAlive() const {
    // TODO consider HTTP/1.0 and HTTP/1.1
    const std::string value = getHeader("Connection");
    return value == "keep-alive";
}

const std::string Response::getContentType() const {
    return getHeader("Content-Type");
}

const std::size_t Response::getContentLength() const {
    const std::string value = getHeader("Content-Length");
    if (value.empty())
        return 0;
    return boost::lexical_cast<std::size_t>(value);
}

const std::string Response::getContent() const {
    return content.value_or("");
}

void Response::setKeepAlive(bool on) {
    setHeader("Connection", on ? "keep-alive" : "close");
}

std::string Response::toBuffer() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}

void Response::setHeader(std::string name, std::string value) {
    auto iter = std::find_if(headers.begin(), headers.end(),
                             [&](auto&& pair_) { return pair_.first == name; });
    if (iter == headers.end()) {
        headers.emplace_back(name, value);
    } else {
        iter->second = value;
    }
}

const std::string Response::getHeader(std::string name) const {
    auto iter = std::find_if(headers.begin(), headers.end(),
                             [&](auto&& pair_) { return pair_.first == name; });
    if (iter == headers.end()) {
        return {};
    } else {
        return iter->second;
    }
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
