#ifndef ACPROXY_HTTP_FORWARD_HANDLE_HPP_
#define ACPROXY_HTTP_FORWARD_HANDLE_HPP_

#include "socket_handle.hpp"
#include "http/request.hpp"
#include <sstream>
#include <memory>

namespace ACProxy {

class HttpHandle;

class HttpForwardHandle
    : public SocketHandle,
      public std::enable_shared_from_this<HttpForwardHandle> {
public:
    HttpForwardHandle(int sk, Http::Request req,
                      int http_fd, std::vector<char>& out)
        : SocketHandle(sk),
          request(req),
          http_fd(http_fd),
          resp(out),
          finished(false) {}

    void onRead() override {
        LOG_ACPROXY_INFO("HttpForwardHandle onRead happens, call SocketHandle::onRead");
        SocketHandle::onRead();
        LOG_ACPROXY_INFO("HttpForwardHandle forward data");
        resp = std::move(in);

        Reactor& reactor = Reactor::getInstance();
        auto http_handle = reactor.getEventHandle(http_fd);
        reactor.register_(http_handle, Event::Write);
    }

    void onWrite() override {
        if (finished) {
            return;
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

        SocketHandle::onWrite(); // FIXME sending part of data may be present
        finished = true;
        
        // register onRead event to read data from upstream
        auto self = shared_from_this();
        Reactor& reactor = Reactor::getInstance();
        reactor.register_(self, Event::Read); // FIXME
    }

private:
    bool finished;
    Http::Request request;
    int http_fd;
    std::vector<char>& resp;
};
}

#endif
