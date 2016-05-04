#pragma once

#include "connection.hpp"
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <memory>

namespace ACProxy {
class Server : private boost::noncopyable {
public:
    explicit Server(const std::string& addr, int port, std::size_t thread_pool_size);

    void run();

private:
    void startAccept();

    void handleAccept(const boost::system::error_code& e);

    void handleStop();

    std::size_t thread_pool_size_;
    boost::asio::io_service io_service_;
    boost::asio::signal_set signals_;
    boost::asio::ip::tcp::acceptor acceptor_;
    ConnectionPtr new_connection_;
};
}
