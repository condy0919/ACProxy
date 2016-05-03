#ifndef ACPROXY_SOCKET_HANDLE_HPP_
#define ACPROXY_SOCKET_HANDLE_HPP_

#include "event_handle.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <vector>

namespace ACProxy {

class SocketHandle : public EventHandle {
public:
    SocketHandle(int fd) : sk(fd) {}

    ~SocketHandle() override;

    int getHandle() const override {
        return sk;
    }

    int onRead() override;
    int onWrite() override;

private:
    int sk;

protected:
    std::vector<char> in; // TODO buffer list
    std::vector<char> out;
};
}


#endif
