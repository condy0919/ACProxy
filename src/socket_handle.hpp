#ifndef ACPROXY_SOCKET_HANDLE_HPP_
#define ACPROXY_SOCKET_HANDLE_HPP_

#include "event_handle.hpp"
#include "reactor.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <map>

namespace ACProxy {

class SocketHandle : public EventHandle {
public:
    SocketHandle(int fd) : sk(fd), buf(new char[MAX_SIZE]) {
        std::memset(buf, 0, MAX_SIZE);
    }

    ~SocketHandle() override {
        ::close(sk);
        delete[] buf; // FIXME
    }

    int getHandle() const {
        return sk;
    }

    void onRead() override {
        int ret = ::recv(sk, buf, MAX_SIZE, 0);
        if (ret > 0) {
            std::printf("received: %s\n", buf);
            ::send(sk, buf, ret, 0);
        }
    }

private:
    int sk;
    char* buf;
    static const int MAX_SIZE = 1024;
};
}


#endif
