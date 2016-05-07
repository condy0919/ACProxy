#pragma once

#include "http/request.hpp"
#include "forward.hpp"
//#include "client_forwarder.hpp"
//#include "server_forwarder.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <array>


namespace ACProxy {
class ClientForwarder;
class ServerForwarder;
class Forwarder;
class Connection : public std::enable_shared_from_this<Connection>,
                   private boost::noncopyable {
    friend class Forwarder; // FIXME
public:
    explicit Connection(boost::asio::io_service& io_service);

    boost::asio::ip::tcp::socket& socket();

    void start();

    boost::asio::io_service& getIOService();

    std::shared_ptr<ServerForwarder> getServerForwarder();

    std::shared_ptr<ClientForwarder> getClientForwarder();

private:
    void handleHeaderRead(const boost::system::error_code& e,
                          std::size_t bytes_transferred);

    void handleBodyRead(const boost::system::error_code& e,
                        std::size_t bytes_transferred);

    void handleReply(const boost::system::error_code& e,
                     std::size_t bytes_transferred);

    void reply(std::string data);

    void forward();

    boost::asio::io_service::strand strand_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf buffer_;
    Http::Request request_;
    std::shared_ptr<Forwarder> fwd_;
    std::size_t content_body_remain = 0;
};
}
