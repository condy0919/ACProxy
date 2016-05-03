#include "http_forward_handle.hpp"
#include "log.hpp"
#include "reactor.hpp"
#include <sstream>
#include <algorithm>

namespace ACProxy {
int HttpForwardHandle::onRead() {
    LOG_ACPROXY_INFO(
        "HttpForwardHandle onRead happens, call SocketHandle::onRead");
    if (!header_finished && !SocketHandle::onRead()) {
        return false;
    }

    if (receiver.expired()) {
        return false;
    }

    LOG_ACPROXY_INFO("after SocketHandle::onRead");
    auto http_handle = receiver.lock();


    // check header of resp
    std::vector<char>::iterator iter;
    Http::ResponseParser::ResultType res;
    std::tie(res, iter) = parser.parse(resp, in.begin(), in.end());
    if (res == Http::ResponseParser::bad) {
        LOG_ACPROXY_ERROR("response parse failed");
        return false;
    } else if (res == Http::ResponseParser::good) {
        LOG_ACPROXY_DEBUG("response parse success");
    } else {
        LOG_ACPROXY_INFO("response indeterminate");
        return true;
    }

    // get Content-Length
    int len = 0;
    for (Http::Header h : resp.headers) {
        if (h.name == "Content-Length") {
            len = std::stoi(h.value);
            break;
        }
    }
    LOG_ACPROXY_INFO("response len = ", len);
    len -= in.size();

    int fd = getHandle();
    static char buf[4096];
    while (len > 0) {
        int ret = ::recv(fd, buf, sizeof(buf), 0);
        if (ret == -1) {
            if (errno == EAGAIN) {
                LOG_ACPROXY_ERROR("YOU GUYS SUCKS");
            }
            break;
        }
        in.insert(in.end(), buf, buf + ret);
        len -= ret;
    }

    LOG_ACPROXY_INFO("HttpForwardHandle forward data");
    http_handle->setResp(std::move(in));

    LOG_ACPROXY_INFO("setResp ok");

    Reactor& reactor = Reactor::getInstance();
    reactor.register_(http_handle, static_cast<Event>(Event::Write | Event::Read));

    LOG_ACPROXY_INFO("register http_handle with write event");
    return true;
}

int HttpForwardHandle::onWrite() {
    std::ostringstream oss;

    oss << request.method << " " << request.uri << " "
        << "HTTP/" << request.http_version_major << "."
        << request.http_version_minor << "\r\n";
    for (Http::Header h : request.headers) {
        oss << h.name << ": " << h.value << "\r\n";
    }
    oss << "\r\n";

    std::string sent = oss.str();
    out.insert(out.end(), sent.begin(), sent.end());
    LOG_ACPROXY_INFO("HttpForwardHandle prepare header");

    SocketHandle::onWrite();  // FIXME sending part of data may be present

    // register onRead event to read data from upstream
    auto self = shared_from_this();
    Reactor& reactor = Reactor::getInstance();
    reactor.register_(self, static_cast<Event>(Event::Read | Event::Write));  // FIXME
    return true;
}
}
