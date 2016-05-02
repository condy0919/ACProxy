#include "socket_handle.hpp"
#include "log.hpp"
#include "reactor.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <algorithm>

namespace ACProxy {

SocketHandle::~SocketHandle() {
    LOG_ACPROXY_WARNING("close socket ", sk);
    ::close(sk);
}

bool SocketHandle::onRead() {
    LOG_ACPROXY_INFO("SocketHandle start to read");
    static char buf[1024];  // TODO CUSTOM IT
    int ret = 0;
    while ((ret = ::recv(sk, buf, sizeof(buf), 0)) > 0) {
        in.insert(in.end(), buf, buf + ret);
    }
    LOG_ACPROXY_INFO("the content received ", in.size(), " bytes");
    //std::for_each(in.begin(), in.end(), std::putchar);
    //LOG_ACPROXY_INFO("content ends");

    if (ret == -1) {
        const int err_code = errno;
        if (err_code == EAGAIN) {
            LOG_ACPROXY_INFO("SocketHandle::onRead EAGAIN happens");
            return true;
        }
        return false;
    } else if (ret == 0) {
        LOG_ACPROXY_INFO("SocketHandle::onRead ends");
        Reactor& reactor = Reactor::getInstance();

        LOG_ACPROXY_INFO("start to remove it");
        reactor.unregister(sk);
        LOG_ACPROXY_INFO("remove done");
        return false;
    } else {
        LOG_ACPROXY_INFO("ret of last recv = ", ret);
        return false;
    }
}

bool SocketHandle::onWrite() {
    if (out.empty()) {
        return false;
    }

    LOG_ACPROXY_INFO("SocketHandle start to write");

    //std::for_each(out.begin(), out.end(), std::putchar);

    size_t sent = 0, total = out.size();
    int ret = 0;
    while (sent < total) {
        ret = ::send(sk, out.data() + sent, total - sent, MSG_NOSIGNAL);
        if (ret < 0) {
            break;
        }
        sent += ret;
    }
    std::vector<char> out_(out.begin() + sent, out.end());
    out_.swap(out);
    if (ret == -1) {
        const int err_code = errno;
        if (err_code == EAGAIN) {
            LOG_ACPROXY_INFO("SocketHandle::onWrite EAGAIN");
        }
        LOG_ACPROXY_INFO("SocketHandle::onWrite ends, errno = ", std::strerror(err_code));
    }
    return true;
}
}
