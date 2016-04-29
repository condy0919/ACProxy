#ifndef ACPROXY_EPOLL_HPP_
#define ACPROXY_EPOLL_HPP_

#include "log.hpp"
#include "event.hpp"
#include "event_handle.hpp"
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <map>
#include <memory>

namespace ACProxy {

class Epoll {
public:
    Epoll() : max_fd(0) {
        epfd = ::epoll_create(1024);
    }

    ~Epoll() {
        ::close(epfd);
    }

    int waitEvent(std::map<int, std::shared_ptr<EventHandle>>& hs,
                   int timeout = 0) {
        std::vector<struct epoll_event> evs(max_fd);
        int n = ::epoll_wait(epfd, &evs[0], max_fd, timeout);
        if (n > 0) {
            for (int i = 0; i < n; ++i) {
                int h = evs[i].data.fd;
                if (evs[i].events & EPOLLERR) {
                    hs[h]->onError();
                } else {
                    if (evs[i].events & EPOLLIN) {
                        hs[h]->onRead();
                    }
                    if (evs[i].events & EPOLLOUT) {
                        hs[h]->onWrite();
                    }
                }
            }
        } else {
            LOG_ACPROXY_ERROR("epoll_wait error ", std::strerror(errno));
        }
        return n;
    }

    int register_(int h, Event evt) {
        struct epoll_event ev = {};
        ev.data.fd = h;
        if (static_cast<int>(evt) & static_cast<int>(Event::Read)) {
            ev.events |= EPOLLIN;
        }
        if (static_cast<int>(evt) & static_cast<int>(Event::Write)) {
            ev.events |= EPOLLOUT;
        }
        ev.events |= EPOLLET;

        if (::epoll_ctl(epfd, EPOLL_CTL_ADD, h, &ev) < 0) {
            const int err_code = errno;
            LOG_ACPROXY_ERROR("epoll_ctl add error ", std::strerror(err_code));
            return -err_code;
        }
        LOG_ACPROXY_INFO("epoll_ctl add ", h, " succeed");
        ++max_fd;
        return 0;
    }

    int modify(int h, Event evt) {
        struct epoll_event ev = {};
        ev.data.fd = h;
        if (static_cast<int>(evt) & static_cast<int>(Event::Read)) {
            ev.events |= EPOLLIN;
        }
        if (static_cast<int>(evt) & static_cast<int>(Event::Write)) {
            ev.events |= EPOLLOUT;
        }
        ev.events |= EPOLLET;
        
        if (::epoll_ctl(epfd, EPOLL_CTL_MOD, h, &ev) < 0) {
            const int err_code = errno;
            LOG_ACPROXY_ERROR("epoll_ctl mod error ", std::strerror(err_code));
            return -err_code;
        }
        LOG_ACPROXY_INFO("epoll_ctl mod ", h, " succeed");
        return 0;
    }

    int unregister(int h) {
        if (::epoll_ctl(epfd, EPOLL_CTL_DEL, h, NULL) < 0) {
            const int err_code = errno;
            LOG_ACPROXY_ERROR("epoll_ctl del ", h, " error ", std::strerror(err_code));
            return -err_code;
        }
        --max_fd;
        return 0;
    }

private:
    int max_fd;
    int epfd;
};
}

#endif
