#include "log.hpp"
#include "server_forwarder.hpp"
#include "client_forwarder.hpp"
#include <boost/bind.hpp>

namespace ACProxy {

ServerForwarder::ServerForwarder(std::observer_ptr<Connection> conn)
    : strand_(conn->getIOService()), conn_(conn) {
    ;  //
}

std::shared_ptr<boost::asio::ip::tcp::socket> ServerForwarder::socket() {
    return socket_;
}

void ServerForwarder::socket(std::shared_ptr<boost::asio::ip::tcp::socket> sock) {
    socket_ = sock;
}

void ServerForwarder::send(std::string data) {
    boost::asio::async_write(
        *socket_, boost::asio::buffer(data.data(), data.size()),
        strand_.wrap(boost::bind(&ServerForwarder::sendHandle,
                                 shared_from_this(),
                                 boost::asio::placeholders::error)));
}

void ServerForwarder::sendHandle(const boost::system::error_code& e) {
    if (!e) {
        LOG_ACPROXY_INFO(
            "send data to upstream success, starting recving http response...");
        //recv_internal();
    } else {
        LOG_ACPROXY_ERROR("send data to upstream error ", e.message());
    }
}
}
