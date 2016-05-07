#pragma once

#include "connection.hpp"
#include "http/request.hpp"
#include "../libs/observer_ptr.hpp"
#include <boost/asio.hpp>
#include <memory>

namespace ACProxy {
//class Connection;
class ClientForwarder : public std::enable_shared_from_this<ClientForwarder>,
                        private boost::noncopyable {
public:
    explicit ClientForwarder(std::observer_ptr<Connection> conn);

    std::shared_ptr<boost::asio::ip::tcp::socket> socket();

    void send(std::string data);

private:
    void sendHandle(const boost::system::error_code& e);

    std::vector<char> headers;
    void getHeaders();
    void getHeadersHandle(const boost::system::error_code& e);

    boost::asio::streambuf buffer_;
    void getBody();
    void getBodyHandle(const boost::system::error_code& e);

private:
    boost::asio::io_service::strand strand_;
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    Http::Request request_;

    std::observer_ptr<Connection> conn_;
};
}
