#pragma once

#include "request_handler.hpp"
#include "http/request.hpp"
#include "http/request_parser.hpp"
#include "http/response.hpp"
#include <memory>
#include <array>
#include <boost/asio.hpp>

namespace ACProxy {
class Connection : public std::enable_shared_from_this<Connection>,
                   private boost::noncopyable {
public:
    explicit Connection(boost::asio::io_service& io_service,
            RequestHandler& handler);

    boost::asio::ip::tcp::socket& socket();

    void start();

private:
    void handleRead(const boost::system::error_code& e,
                    std::size_t bytes_transferred);

    void handleForward(const boost::system::error_code& e);

    boost::asio::io_service::strand strand_;
    boost::asio::ip::tcp::socket socket_;
    std::array<char, 8192> buffer_; // TODO
    RequestHandler& request_handler_;
    Http::Request request_;
    Http::RequestParser request_parser_;
    Http::Response resp_;
};

using ConnectionPtr = std::shared_ptr<Connection>;
}
