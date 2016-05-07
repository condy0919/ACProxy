#pragma once

#include "log.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <memory>
#include <vector>

namespace ACProxy {
class Connection;

class Forwarder : public std::enable_shared_from_this<Forwarder>,
                  private boost::noncopyable {
public:
    Forwarder(std::shared_ptr<Connection> conn, Http::Request req);

    void start();

private:
    void handleRequestWrite(const boost::system::error_code& e,
                            std::size_t bytes_transferred);

    void handleHeaderRead(const boost::system::error_code& e,
                          std::size_t bytes_transferred);

    void handleBodyRead(const boost::system::error_code& e,
                        std::size_t bytes_transferred);

    void forwardBodyToClient();

    void forwardHeaderToClient();

private:
    std::shared_ptr<Connection> conn_;
    boost::asio::io_service& service_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::io_service::strand strand_;
    boost::asio::streambuf buffer_;
    Http::Request request_;
    Http::Response response_;
    std::size_t content_body_remain = 0;
};
}
