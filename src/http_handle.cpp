#include "http_handle.hpp"
#include "http_forward_handle.hpp"
#include "reactor.hpp"
#include "log.hpp"
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>

namespace ACProxy {
int HttpHandle::onRead() {
    if (!SocketHandle::onRead()) {  // prepare http request
        return false;
    }

    Http::RequestParser::ResultType res;
    std::tie(res, std::ignore) =
        request_parser.parse(request, in.begin(), in.end());
    if (res == Http::RequestParser::good) {
        LOG_ACPROXY_INFO("http request parsed correct");
        std::cout << request << '$';
        // filter
        // cache
        // forward req to source

        // TODO Simplify it
        std::string host;
        int port = 80;

        // TODO support more methods
        /*
        if (request.method == "CONNECT") {
            int pos = request.uri.find_first_of(":");
            if (pos == std::string::npos) {
                host = request.uri;
            } else {
                host = request.uri.substr(0, pos);
                port = std::stoi(request.uri.substr(pos + 1));
            }
        } else { */
            for (Http::Header h : request.headers) {
                if (h.name == "Host") {
                    int pos = h.value.find_first_of(":");
                    if (pos == std::string::npos) {
                        host = h.value;
                    } else {
                        host = h.value.substr(0, pos);
                        std::string s = h.value.substr(pos + 1);
                        port = std::stoi(h.value.substr(pos + 1));
                    }
                    break;
                }
            }
        //}
        LOG_ACPROXY_INFO("host = ", host);
        struct hostent* h = ::gethostbyname(host.data());
        if (!h) {
            const int err_code = errno;
            LOG_ACPROXY_ERROR("gethostbyname error ", std::strerror(err_code));
            return false;
        }
        int total = 0;
        for (int i = 0; h->h_addr_list[i]; ++i) {
            ++total;
        }
        int idx = std::rand() % total;  // TODO use std::random later
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(port);  // FIXME
        addr.sin_addr = *(struct in_addr*)h->h_addr_list[idx];

        // TODO connection-pool

        int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (fd < 0) {
            const int err_code = errno;
            LOG_ACPROXY_ERROR("socket error ", std::strerror(err_code));
        }

        ::connect(fd, (struct sockaddr*)&addr, sizeof(addr));

        LOG_ACPROXY_INFO("HttpForwardHandle created");
        auto self = shared_from_this();
        auto hd = std::make_shared<HttpForwardHandle>(fd, request, self);

        LOG_ACPROXY_INFO("register forward in reactor");
        Reactor& reactor = Reactor::getInstance();
        reactor.register_(hd, static_cast<Event>(Event::Write | Event::Read));

        request_parser.reset(); // for next http request
        request = Http::Request();
        in.clear();

        return true;
    } else if (res == Http::RequestParser::bad) {
        LOG_ACPROXY_ERROR("http header parse bad");
        return false;
    } else {  // indetermined
        LOG_ACPROXY_ERROR("http header parse indetermined");
        return false;
    }
}
}
