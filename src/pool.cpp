#include "pool.hpp"

namespace ACProxy {
std::shared_ptr<boost::asio::ip::tcp::socket> SocketPool::get(std::string ip,
                                                              int port) {
    // TODO implement
    return {};
}

void SocketPool::put(std::string ip, int port,
                     std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    // TODO implement
    return;
}
}
