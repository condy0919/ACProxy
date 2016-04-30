#ifndef ACPROXY_HTTP_FORWARD_HANDLE_HPP_
#define ACPROXY_HTTP_FORWARD_HANDLE_HPP_

#include "socket_handle.hpp"
#include "http_handle.hpp"
#include "http/request.hpp"
#include <memory>

namespace ACProxy {

class HttpForwardHandle
    : public SocketHandle,
      public std::enable_shared_from_this<HttpForwardHandle> {
public:
    HttpForwardHandle(int sk, Http::Request req, std::weak_ptr<HttpHandle> hptr)
        : SocketHandle(sk), request(req), receiver(hptr), finished(false) {}

    bool onRead() override;
    bool onWrite() override;

private:
    bool finished;
    Http::Request request;
    std::weak_ptr<HttpHandle> receiver;
};
}

#endif
