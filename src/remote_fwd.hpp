#pragma once

#include "connection.hpp"
#include "http/response.hpp"
#include "../libs/observer_ptr.hpp"
#include <boost/noncopyable.hpp>
#include <string>
#include <memory>

namespace ACProxy {

class RemoteForwarder : public std::enable_shared_from_this<RemoteForwarder>,
                        private boost::noncopyable {
public:
    explicit RemoteForwarder(std::observer_ptr<Connection> conn);

    ~RemoteForwarder() noexcept;

    std::shared_ptr<boost::asio::ip::tcp::socket> socket();
    void socket(std::shared_ptr<boost::asio::ip::tcp::socket> sock);

    void connect(std::string host, int port);

    void setResponseBody(bool on = true) noexcept;
    void setParseResponseHeader(bool on = true) noexcept;

    void send(std::string data);

private:
    bool has_response_body_ = true;
    bool parse_response_header_ = true;
    bool get_header_once_ = true;

private:
    void sendHandle(const boost::system::error_code& e);

    std::array<char, 8192> raw_data_;
    void getRawData();
    void getRawDataHandle(const boost::system::error_code& e,
                          std::size_t bytes_transferred);

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
    Http::Response response_;

    std::observer_ptr<Connection> conn_;
};
}
