#pragma once

#include "http/request.hpp"
#include "forward.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <array>

namespace ACProxy {
class Connection : public std::enable_shared_from_this<Connection>,
                   private boost::noncopyable {
    friend class Forwarder; // FIXME
public:
    explicit Connection(boost::asio::io_service& io_service);

    boost::asio::ip::tcp::socket& socket();

    void start();

private:
    void handleHeaderRead(const boost::system::error_code& e,
                          std::size_t bytes_transferred);

    void handleBodyRead(const boost::system::error_code& e,
                        std::size_t bytes_transferred);

    // used in Forwarder
    void handleWrite(const boost::system::error_code& e,
                     std::size_t bytes_transferred);

    void forward();

    boost::asio::io_service::strand strand_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf buffer_;
    Http::Request request_;
    std::shared_ptr<Forwarder> fwd_;
};
}
