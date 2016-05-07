#include "log.hpp"
#include "local_fwd.hpp"
#include "remote_fwd.hpp"
#include "connection.hpp"
#include "../libs/observer_ptr.hpp"
#include <boost/bind.hpp>
#include <functional>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <vector>

namespace ACProxy {
Connection::Connection(boost::asio::io_service& io_service)
    : io_service_(io_service),
      local_fwd_(std::make_shared<LocalForwarder>(this)),
      remote_fwd_(std::make_shared<RemoteForwarder>(this)) {}

Connection::~Connection() noexcept {
    LOG_ACPROXY_INFO("Connection is freed, CAUTIOUS");
}

boost::asio::ip::tcp::socket& Connection::socket() {
    return *local_fwd_->socket();
}

void Connection::start() {
    //auto self = shared_from_this();
    //local_fwd_ = std::make_shared<LocalForwarder>(self);
    //remote_fwd_ = std::make_shared<RemoteForwarder>(self);

    local_fwd_->start();
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
}
