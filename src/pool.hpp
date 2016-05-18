#pragma once

#include "cache.hpp"
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <memory>

namespace ACProxy {

class SocketPool : private boost::noncopyable {
public:
    std::shared_ptr<boost::asio::ip::tcp::socket> get(std::string ip, int port);

    void put(std::string ip, int port,
             std::shared_ptr<boost::asio::ip::tcp::socket> socket);

private:
    LocalCache<std::string, std::shared_ptr<boost::asio::ip::tcp::socket>, 512>
        cache_;
};
}
