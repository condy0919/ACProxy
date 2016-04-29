#ifndef ACPROXY_HTTPHANDLE_HPP_
#define ACPROXY_HTTPHANDLE_HPP_

#include "http_forward_handle.hpp"
#include "socket_handle.hpp"
#include "http/request_parser.hpp"
#include "http/request.hpp"
#include "http/header.hpp"
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <memory>

namespace ACProxy {

class HttpHandle : public SocketHandle {
                   
public:
    HttpHandle(int fd) : sk(fd), SocketHandle(fd) {}

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

            // TODO Simplify it
            std::string host;
            for (Http::Header h : request.headers) {
                if (h.name == "Host") {
                    int pos = h.value.find_first_of(":");
                    host = h.value.substr(0, pos);
                    //host = h.value;
                    break;
                }
            }
            LOG_ACPROXY_INFO("host = ", host);
            struct hostent* h = ::gethostbyname(host.data());
            if (!h) {
                const int err_code = errno;
                LOG_ACPROXY_ERROR("gethostbyname error ", std::strerror(err_code));
            }
            int total = 0;
            for (int i = 0; h->h_addr_list[i]; ++i) {
                ++total;
            }
            int idx = std::rand() % total; // TODO use std::random later
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = ::htons(80); // FIXME 
            addr.sin_addr = *(struct in_addr*)h->h_addr_list[idx];

            int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
            if (fd < 0) {
                const int err_code = errno;
                LOG_ACPROXY_ERROR("socket error ", std::strerror(err_code));
            }
            //int flags = ::fcntl(fd, F_GETFL, 0); // FIXME check return value
            //::fcntl(fd, F_SETFL, flags | O_NONBLOCK);

            ::connect(fd, (struct sockaddr*)&addr, sizeof(addr));

            LOG_ACPROXY_INFO("HttpForwardHandle created");
            auto hd = std::make_shared<HttpForwardHandle>(fd, request, sk, out);

            LOG_ACPROXY_INFO("register forward in reactor");
            Reactor& reactor = Reactor::getInstance();
            reactor.register_(hd, Event::Write);

        } else if (res == Http::RequestParser::bad) {
            LOG_ACPROXY_ERROR("http header parse bad");
        } else { // indetermined
            LOG_ACPROXY_ERROR("http header parse indetermined");
        }
    }

private:
    int sk;
    Http::RequestParser request_parser;
    Http::Request request;
};
}

#endif
