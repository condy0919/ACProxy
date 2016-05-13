#include "log.hpp"
#include "local_fwd.hpp"
#include "remote_fwd.hpp"
#include "connection.hpp"
#include "connection_mgr.hpp"
#include <boost/bind.hpp>
#include <functional>
#include <algorithm>
#include <sstream>
#include <cstring>

namespace ACProxy {

Connection::Connection(boost::asio::io_service& io_service,
                       ConnectionManager& mgr)
    : io_service_(io_service),
      conn_mgr_(mgr),
      local_fwd_(std::make_shared<LocalForwarder>(this)),
      remote_fwd_(std::make_shared<RemoteForwarder>(this)),
      timeout_(io_service) {
    timeout_.expires_at(boost::posix_time::pos_infin);
}

Connection::~Connection() noexcept {
    stop();
    LOG_ACPROXY_INFO("Connection is freed, CAUTIOUS");
}

boost::asio::ip::tcp::socket& Connection::socket() {
    return *local_fwd_->socket();
}

void Connection::start() {
    update();
    timeout_.async_wait(boost::bind(&Connection::timeout, shared_from_this(),
                                    boost::asio::placeholders::error));
    local_fwd_->start();
}

void Connection::stop() {
    local_fwd_->stop();
    remote_fwd_->stop();
}

void Connection::update() {
    // XXX HARD CODE
    timeout_.expires_from_now(boost::posix_time::seconds(10));
}

boost::asio::io_service& Connection::getIOService() {
    return io_service_;
}

std::shared_ptr<RemoteForwarder> Connection::getRemoteForwarder() {
    return remote_fwd_;
}

std::shared_ptr<LocalForwarder> Connection::getLocalForwarder() {
    return local_fwd_;
}

void Connection::timeout(const boost::system::error_code& e) {
    if (e == boost::asio::error::operation_aborted) {
        timeout_.async_wait(boost::bind(&Connection::timeout,
                                        shared_from_this(),
                                        boost::asio::placeholders::error));
        return;
    }

    // timeout occurs
    LOG_ACPROXY_INFO("connection timeout 10s, need to be closed");
    // stop this connection
    conn_mgr_.stop(shared_from_this());  // FIXME
    LOG_ACPROXY_INFO("connection stop success");
}
}
