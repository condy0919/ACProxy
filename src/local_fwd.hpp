#pragma once

#include "connection.hpp"
#include "http/request.hpp"
#include "../libs/observer_ptr.hpp"
#include <boost/asio.hpp>
#include <memory>

namespace ACProxy {

class LocalForwarder : public std::enable_shared_from_this<LocalForwarder>,
                        private boost::noncopyable {
public:
    explicit LocalForwarder(std::observer_ptr<Connection> conn);
    //explicit LocalForwarder(std::shared_ptr<Connection> conn);
    ~LocalForwarder() noexcept;

    std::shared_ptr<boost::asio::ip::tcp::socket> socket();

    void start();

    void send(std::string data);

private:
    void sendHandle(const boost::system::error_code& e);

    std::vector<char> headers;
    void getHeaders();
    void getHeadersHandle(const boost::system::error_code& e);

    boost::asio::streambuf buffer_;
    void getBody();
    void getBodyHandle(const boost::system::error_code& e,
                       std::size_t bytes_transferred);

private:
    boost::asio::io_service::strand strand_;
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    Http::Request request_;

    std::observer_ptr<Connection> conn_;
    //std::shared_ptr<Connection> conn_;
};
}
