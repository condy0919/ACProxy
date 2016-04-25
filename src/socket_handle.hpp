#ifndef ACPROXY_SOCKET_HANDLE_HPP_
#define ACPROXY_SOCKET_HANDLE_HPP_

#include "event_handle.hpp"
#include "reactor.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <map>
#include <algorithm>

namespace ACProxy {

class SocketHandle : public EventHandle {
public:
    SocketHandle(int fd) : sk(fd) {}

    ~SocketHandle() override {
        ::close(sk);
    }

    int getHandle() const override {
        return sk;
    }

    void onRead() override {
        static char buf[1024];
        int ret = 0;
        while ((ret = ::recv(sk, buf, sizeof(buf), 0)) != -1) {
            in.insert(in.end(), buf, buf + ret);
        }
        LOG_ACPROXY_INFO("the content received");
        std::for_each(in.begin(), in.end(), std::putchar);
        LOG_ACPROXY_INFO("content ends");

        const int err_code = errno;
        if (err_code == EAGAIN) {
            return;
        } else {
            LOG_ACPROXY_ERROR("error occured in SocketHandle::onRead errno = ",
                              err_code);
        }
    }

    void onWrite() override {
        if (out.empty()) {
            return;
        }

        size_t sent = 0;
        int ret = 0;
        while ((ret = ::send(sk, out.data() + sent, out.size(), MSG_NOSIGNAL)) > 0) {
            sent += ret;
        }
        const int err_code = errno;
        if (ret == -1) {
            if (err_code == EAGAIN && sent < out.size()) {
                std::vector<char> out_(out.begin() + sent, out.end());
                out_.swap(out);
            }
        }
    }

private:
    int sk;

protected:
    std::vector<char> in; // TODO buffer list
    std::vector<char> out;
};
}


#endif
