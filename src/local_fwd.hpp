#pragma once

#include "connection.hpp"
#include "http/request.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <mutex>

namespace ACProxy {

class LocalForwarder : public std::enable_shared_from_this<LocalForwarder>,
                       private boost::noncopyable {
public:
    explicit LocalForwarder(std::shared_ptr<Connection> conn);
    ~LocalForwarder() noexcept;

    std::shared_ptr<boost::asio::ip::tcp::socket> socket();

    void start();

    void stop();

    void send(std::string data);
    void finish(std::string data);

private:
    void sendHandle(const boost::system::error_code& e);
    void finishHandle(const boost::system::error_code& e);

    std::vector<char> headers;
    void getHeaders();
    void getHeadersHandle(const boost::system::error_code& e);

    boost::asio::streambuf buffer_;
    void getBody();
    void getBodyHandle(const boost::system::error_code& e,
                       std::size_t bytes_transferred);

private:
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    Http::Request request_;

    std::once_flag close_flag_;

    std::weak_ptr<Connection> conn_;
};
}
