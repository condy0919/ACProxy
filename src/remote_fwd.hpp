#pragma once

#include "connection.hpp"
#include "http/response.hpp"
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <string>
#include <memory>
#include <mutex>

namespace ACProxy {

class RemoteForwarder : public std::enable_shared_from_this<RemoteForwarder>,
                        private boost::noncopyable {
public:
    explicit RemoteForwarder(std::shared_ptr<Connection> conn);

    ~RemoteForwarder() noexcept;

    void stop();

    std::shared_ptr<boost::asio::ip::tcp::socket> socket();
    void socket(std::shared_ptr<boost::asio::ip::tcp::socket> sock);

    bool connect(std::string host, int port);

    void setResponseBody(bool on = true) noexcept;
    void setParseResponseHeader(bool on = true) noexcept;
    void setCacheKey(std::string key);

    void send(std::string data);

private:
    bool has_response_body_ = true;
    bool parse_response_header_ = true;
    bool get_header_once_ = true;
    boost::optional<std::string> cache_key_;
    std::string cache_value_;

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
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    Http::Response response_;

    std::once_flag close_flag_;

    std::weak_ptr<Connection> conn_;
};
}
