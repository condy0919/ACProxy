#ifndef ACPROXY_HTTP_FORWARD_HANDLE_HPP_
#define ACPROXY_HTTP_FORWARD_HANDLE_HPP_

#include "socket_handle.hpp"
#include "http_handle.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "http/response_parser.hpp"
#include <memory>

namespace ACProxy {

class HttpForwardHandle
    : public SocketHandle,
      public std::enable_shared_from_this<HttpForwardHandle> {
public:
    HttpForwardHandle(int sk, Http::Request req, std::weak_ptr<HttpHandle> hptr)
        : SocketHandle(sk), request(req), receiver(hptr), header_finished(false) {}

    int onRead() override;
    int onWrite() override;

    std::string getName() const override {
        return "HttpForwardHandle";
    }

private:
    bool header_finished;
    Http::ResponseParser parser;
    Http::Request request;
    Http::Response resp;
    std::weak_ptr<HttpHandle> receiver;
};
}

#endif
