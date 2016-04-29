#ifndef ACPROXY_REACTOR_HPP_
#define ACPROXY_REACTOR_HPP_

#include "event.hpp"
#include "event_handle.hpp"
#include "epoll.hpp"
#include <boost/noncopyable.hpp>

namespace ACProxy {

class Reactor : public boost::noncopyable {
public:
    Reactor() = default;

    static Reactor& getInstance() {
        static thread_local Reactor reactor;
        return reactor;
    }

    int register_(std::shared_ptr<EventHandle> hd, Event evt) {
        int h = hd->getHandle();
        if (handlers.find(h) == handlers.end()) {
            handlers.insert(std::make_pair(h, hd));
            return epoll.register_(h, evt);
        } else {
            return epoll.modify(h, evt);
        }
    }

    void unregister(std::shared_ptr<EventHandle> hd) {
        LOG_ACPROXY_INFO("get fd");
        int h = hd->getHandle();
        LOG_ACPROXY_INFO("epoll remove");
        epoll.unregister(h);
        LOG_ACPROXY_INFO("handlers erase");
        handlers.erase(h);
        LOG_ACPROXY_INFO("all done");
    }

    void eventLoop(int timeout = 0) {
        epoll.waitEvent(handlers, timeout);
    }

    std::shared_ptr<EventHandle> getEventHandle(int fd) {
        auto iter = handlers.find(fd);
        if (iter == handlers.end())
            return nullptr;
        return iter->second;
    }

private:
    Epoll epoll;
    std::map<int, std::shared_ptr<EventHandle>> handlers;
};
}


#endif
