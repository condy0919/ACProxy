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
        }
        return epoll.register_(h, evt);
    }

    void remove(std::shared_ptr<EventHandle> hd) {
        int h = hd->getHandle();
        epoll.remove(h);
        handlers.erase(h);
    }

    void eventLoop(int timeout = 0) {
        epoll.wait_event(handlers, timeout);
    }

private:
    Epoll epoll;
    std::map<int, std::shared_ptr<EventHandle>> handlers;
};
}


#endif
