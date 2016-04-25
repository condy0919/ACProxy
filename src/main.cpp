#include "log.hpp"
#include "event.hpp"
#include "event_handle.hpp"
#include "epoll.hpp"
#include "reactor.hpp"
#include "listen_handle.hpp"
#include "socket_handle.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace ACProxy;

int main() {
    int sk = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sk < 0) {
        const int err_code = errno;
        DEBUG_THROW_SYSTEMEXCEPTION("socket error ", strerror(err_code));
    }

    const int yes = 1;
    ::setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(8086);
    addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    if (::bind(sk, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        const int err_code = errno;
        DEBUG_THROW_SYSTEMEXCEPTION("bind error ", strerror(err_code));
    }

    if (listen(sk, 65536) < 0) {
        const int err_code = errno;
        DEBUG_THROW_SYSTEMEXCEPTION("listen error ", strerror(err_code));
    }

    // TODO threadpool
    Reactor& reactor = Reactor::getInstance();
    std::shared_ptr<EventHandle> hd = std::make_shared<ListenHandle>(sk);
    reactor.register_(hd, Event::Read);
    while (true) {
        reactor.eventLoop(-1);
        std::printf("one loop\n");
    }
    return 0;
}
