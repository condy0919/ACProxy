#ifndef ACPROXY_HTTPHANDLE_HPP_
#define ACPROXY_HTTPHANDLE_HPP_

#include "socket_handle.hpp"
#include "http/request_parser.hpp"
#include "http/request.hpp"

namespace ACProxy {

class HttpHandle : public SocketHandle {
public:
    using SocketHandle::SocketHandle;

    void onRead() override {
        SocketHandle::onRead(); // prepare http request

        Http::RequestParser::ResultType res;
        std::tie(res, std::ignore) =
            request_parser.parse(request, in.begin(), in.end());
        if (res == Http::RequestParser::good) {
            LOG_ACPROXY_INFO("http request parsed correct");
            // filter 
            // cache
            // forward req to source
        } else if (res == Http::RequestParser::bad) {

        } else {

        }
    }

    void onWrite() override {

    }

private:
    Http::RequestParser request_parser;
    Http::Request request;
};
}

#endif
