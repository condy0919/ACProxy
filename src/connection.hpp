#pragma once

#include "http/request.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <array>

namespace ACProxy {
class Connection : public std::enable_shared_from_this<Connection>,
                   private boost::noncopyable {
public:
    explicit Connection(boost::asio::io_service& io_service);

    boost::asio::ip::tcp::socket& socket();

    void start();

private:
    void handleHeaderRead(const boost::system::error_code& e,
                          std::size_t bytes_transferred);

    void handleBodyRead(const boost::system::error_code& e,
                        std::size_t bytes_transferred);

    void handleForward(
        const boost::system::error_code& e, std::size_t bytes_transferred);

    boost::asio::io_service::strand strand_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf buffer_;
    Http::Request request_;
};

using ConnectionPtr = std::shared_ptr<Connection>;
}
