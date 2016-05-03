#ifndef ACPROXY_HTTPHANDLE_HPP_
#define ACPROXY_HTTPHANDLE_HPP_

#include "socket_handle.hpp"
#include "http/request_parser.hpp"
#include "http/request.hpp"

#include <memory>

namespace ACProxy {

class HttpHandle : public SocketHandle,
                   public std::enable_shared_from_this<HttpHandle> {
    friend class HttpForwardHandle;
public:
    using SocketHandle::SocketHandle;
    int onRead() override;

    std::string getName() const override {
        return "HttpHandle";
    }

private:
    void setResp(std::vector<char> resp) {
        out = std::move(resp);
    }

private:
    Http::RequestParser request_parser;
    Http::Request request;
};
}

#endif
