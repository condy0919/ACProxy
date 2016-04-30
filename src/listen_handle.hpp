#ifndef ACPROXY_LISTEN_HANDLE_HPP_
#define ACPROXY_LISTEN_HANDLE_HPP_

#include "event_handle.hpp"

namespace ACProxy {

class ListenHandle : public EventHandle {
public:
    ListenHandle(int fd) : lfd(fd) {}
    ~ListenHandle() noexcept;

    int getHandle() const noexcept override {
        return lfd;
    }
    bool onRead() override;

private:
    int lfd;
};
}


#endif
