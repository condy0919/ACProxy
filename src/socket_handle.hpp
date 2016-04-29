#ifndef ACPROXY_SOCKET_HANDLE_HPP_
#define ACPROXY_SOCKET_HANDLE_HPP_

#include "event_handle.hpp"
#include "reactor.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <map>
#include <algorithm>
#include <memory>

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
        LOG_ACPROXY_INFO("SocketHandle start to read");

        static char buf[1024]; // TODO CUSTOM IT
        int ret = 0;
        while ((ret = ::recv(sk, buf, sizeof(buf), 0)) > 0) {
            in.insert(in.end(), buf, buf + ret);
        }
        LOG_ACPROXY_INFO("the content received");
        std::for_each(in.begin(), in.end(), std::putchar);
        LOG_ACPROXY_INFO("content ends");

        const int err_code = errno;
        if (ret == -1 && err_code == EAGAIN) {
            LOG_ACPROXY_INFO("SocketHandle::onRead EAGAIN happens");
            return;
        } else if (ret == 0) {
            LOG_ACPROXY_INFO("SocketHandle::onRead ends, errno = ",
                             std::strerror(err_code));
            //Reactor& reactor = Reactor::getInstance();

            //LOG_ACPROXY_INFO("start to remove it");
            //auto self = SocketHandle::shared_from_this();
            //LOG_ACPROXY_WARNING("fd = ", self->sk);
            //reactor.remove(self);
            //LOG_ACPROXY_INFO("remove done");
            ::close(sk);
            LOG_ACPROXY_INFO("SocketHandle::onRead close socket ", sk);
        } else {
            LOG_ACPROXY_ERROR("unknown error ", std::strerror(err_code));
        }
    }

    void onWrite() override {
        LOG_ACPROXY_INFO("SocketHandle::onWrite runs, ", out.empty());
        if (out.empty()) {
            return;
        }

        LOG_ACPROXY_INFO("SocketHandle start to write");

        size_t sent = 0;
        int ret = 0;
        while ((ret = ::send(sk, out.data() + sent, out.size() - sent,
                             MSG_NOSIGNAL)) > 0) {
            sent += ret;
        }
        const int err_code = errno;
        if (ret == -1) {
            if (err_code == EAGAIN && sent <= out.size()) {
                std::vector<char> out_(out.begin() + sent, out.end());
                out_.swap(out);
            }
        }
        LOG_ACPROXY_INFO("SocketHandle::onWrite ends");
    }

private:
    int sk;

protected:
    std::vector<char> in; // TODO buffer list
    std::vector<char> out;
};
}


#endif
