#pragma once

#include "connection.hpp"
#include "http/response.hpp"
#include "../libs/observer_ptr.hpp"
#include <boost/noncopyable.hpp>
#include <string>
#include <memory>

namespace ACProxy {

class ServerForwarder : public std::enable_shared_from_this<ServerForwarder>,
                        private boost::noncopyable {
public:
    explicit ServerForwarder(std::observer_ptr<Connection> conn);

    std::shared_ptr<boost::asio::ip::tcp::socket> socket();
    void socket(std::shared_ptr<boost::asio::ip::tcp::socket> sock);


//private:
    void send(std::string data);
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
    Http::Response response_;

    std::observer_ptr<Connection> conn_;
};
}
