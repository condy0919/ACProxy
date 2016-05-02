#include "request.hpp"

namespace Http {
std::ostream& operator<<(std::ostream& os, Request req) {
    os << req.method << " " << req.uri << " "
       << "HTTP/" << req.http_version_major << "." << req.http_version_minor
       << "\r\n";
    for (Header h : req.headers) {
        os << h.name << ": " << h.value << "\r\n";
    }
    return os << "\r\n";
}
}
