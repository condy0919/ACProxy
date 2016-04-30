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

namespace ACProxy {
bool HttpHandle::onRead() {
    if (!SocketHandle::onRead()) {  // prepare http request
        return false;
    }

    Http::RequestParser::ResultType res;
    std::tie(res, std::ignore) =
        request_parser.parse(request, in.begin(), in.end());
    if (res == Http::RequestParser::good) {
        //const char* resp = "HTTP/1.1 200 OK\r\n"
        //                   "Server: nginx/1.10.0\r\n"
        //                   "Date: Sat, 30 Apr 2016 14:13:21 GMT\r\n"
        //                   "Content-Type: text/html; charset=UTF-8\r\n"
        //                   "Transfer-Encoding: chunked\r\n"
        //                   "Connection: keep-alive\r\n"
        //                   "X-Powered-By: PHP/7.0.6\r\n"
        //                   "\r\n"
        //                   "it's %d\r\n"
        //                   "\r\n";
        //static int cnt = 1;
        //static char buf[1024];
        //std::sprintf(buf, resp, cnt);
        //cnt += 10;

        //LOG_ACPROXY_INFO("start to simulate resp");
        //setResp(std::vector<char>(buf, buf + std::strlen(buf)));
        //{
        //    Reactor& reactor = Reactor::getInstance();
        //    reactor.register_(shared_from_this(), static_cast<Event>(/*Event::Read |*/ Event::Write));
        //    request_parser.reset(); // for next http request
        //    request = Http::Request();
        //}
        //LOG_ACPROXY_INFO("simulatation done");
        ////
        //// test
        ////
        //return true;

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
                // host = h.value;
                break;
            }
        }
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
        addr.sin_port = ::htons(80);  // FIXME
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
