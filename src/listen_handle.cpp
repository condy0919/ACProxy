#include "listen_handle.hpp"
#include "reactor.hpp"
#include "http_handle.hpp"
#include "exception.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

namespace ACProxy {
ListenHandle::~ListenHandle() {
    ::close(lfd);
}

bool ListenHandle::onRead() {
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int fd = ::accept(lfd, (struct sockaddr*)&client, &len);
    if (fd < 0) {
        DEBUG_THROW_SYSTEMEXCEPTION("ListenHandle accept failed");
        return false;
    }
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        LOG_ACPROXY_ERROR("fcntl get flags error");
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) != 0) {
        LOG_ACPROXY_ERROR("fcntl set flags error");
    }
    auto hd = std::make_shared<HttpHandle>(fd);
    // auto hd = std::make_shared<SocketHandle>(fd);

    Reactor& reactor = Reactor::getInstance();
    reactor.register_(hd, static_cast<Event>(Event::Read | Event::Write));
    return true;
}
}
