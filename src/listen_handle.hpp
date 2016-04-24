#ifndef ACPROXY_LISTEN_HANDLE_HPP_
#define ACPROXY_LISTEN_HANDLE_HPP_

#include "socket_handle.hpp"
#include "reactor.hpp"
#include "event_handle.hpp"
#include "exception.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace ACProxy {

class ListenHandle : public EventHandle {
public:
    ListenHandle(int fd) : lfd(fd) {}

    ~ListenHandle() noexcept {
        ::close(lfd);
    }

    int getHandle() const noexcept override {
        return  lfd;
    }

    void onRead() override {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int fd = ::accept(lfd, (struct sockaddr*)&client, &len);
        if (fd < 0) {
            DEBUG_THROW_SYSTEMEXCEPTION("ListenHandle accept failed");
        }
        auto hd = std::make_shared<SocketHandle>(fd);

        Reactor& reactor = Reactor::getInstance();
        reactor.register_(hd, Event::Read);
    }

private:
    int lfd;
};
}


#endif
