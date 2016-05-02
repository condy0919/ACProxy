#include "http_forward_handle.hpp"
#include "log.hpp"
#include "reactor.hpp"
#include <sstream>

namespace ACProxy {
bool HttpForwardHandle::onRead() {
    LOG_ACPROXY_INFO(
        "HttpForwardHandle onRead happens, call SocketHandle::onRead");
    if (!SocketHandle::onRead()) {
        return false;
    }

    //for (char c : in) {
    //    std::putchar(c);
    //}

    LOG_ACPROXY_INFO("after SocketHandle::onRead");
    auto http_handle = receiver.lock();
    LOG_ACPROXY_DEBUG("http_handle.use_count() = ", http_handle.use_count());

    LOG_ACPROXY_INFO("HttpForwardHandle forward data");
    http_handle->setResp(std::move(in));

    LOG_ACPROXY_INFO("setResp ok");

    Reactor& reactor = Reactor::getInstance();
    reactor.register_(http_handle, static_cast<Event>(Event::Write | Event::Read));

    LOG_ACPROXY_INFO("register http_handle with write event");
    return true;
}

bool HttpForwardHandle::onWrite() {
    if (finished) {
        return false;
    }

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
    finished = true;

    // register onRead event to read data from upstream
    auto self = shared_from_this();
    Reactor& reactor = Reactor::getInstance();
    reactor.register_(self, static_cast<Event>(Event::Read | Event::Write));  // FIXME
    return true;
}
}
