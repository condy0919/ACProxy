#ifndef ACPROXY_EPOLL_HPP_
#define ACPROXY_EPOLL_HPP_

#include "event.hpp"
#include "event_handle.hpp"
#include <map>
#include <memory>

namespace ACProxy {

class Epoll {
public:
    Epoll();
    ~Epoll();

    int waitEvent(std::map<int, std::shared_ptr<EventHandle>>& hs,
                  int timeout = 0);
    int register_(int h, Event evt);
    int modify(int h, Event evt);
    int unregister(int h);

private:
    int max_fd;
    int epfd;
};
}

#endif
