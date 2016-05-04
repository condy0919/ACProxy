#include "request_handler.hpp"
#include "connection.hpp"
#include <boost/bind.hpp>
#include <functional>
#include <vector>

namespace ACProxy {
Connection::Connection(boost::asio::io_service& io_service,
                       RequestHandler& handler)
    : strand_(io_service), socket_(io_service), request_handler_(handler) {}

boost::asio::ip::tcp::socket& Connection::socket() {
    return socket_;
}

void Connection::start() {
    socket_.async_read_some(boost::asio::buffer(buffer_),
                            strand_.wrap(boost::bind(
                                &Connection::handleRead, shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred)));
}

void Connection::handleRead(const boost::system::error_code& e,
                            std::size_t bytes_transferred) {
    if (e) {
        return;
    }

    Http::RequestParser::ResultType result;
    std::tie(result, std::ignore) = request_parser_.parse(
        request_, buffer_.data(), buffer_.data() + bytes_transferred);

    if (result) {
        // handle forward
    } else if (!result) {
        // handle bad request
    } else {
        // still read
        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            strand_.wrap(
                boost::bind(&Connection::handleRead, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred)));
    }
}

void Connection::handleForward(const boost::system::error_code& e) {
    if (!e) {
        boost::system::error_code ignored;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored);
    }
}
}
