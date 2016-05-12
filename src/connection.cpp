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
      strand_(io_service),
      conn_mgr_(mgr),
      local_fwd_(std::make_shared<LocalForwarder>(strand_, this)),
      remote_fwd_(std::make_shared<RemoteForwarder>(strand_, this)),
      // is_finished_(false),
      timeout_(io_service) {
    timeout_.expires_at(boost::posix_time::pos_infin);
    timeout();
}

Connection::~Connection() noexcept {
    stop();
    LOG_ACPROXY_INFO("Connection is freed, CAUTIOUS");
}

boost::asio::ip::tcp::socket& Connection::socket() {
    return *local_fwd_->socket();
}

void Connection::start() {
    local_fwd_->start();
    update();
}

void Connection::stop() {
    close();
}

void Connection::update() {
    // XXX HARD CODE
    timeout_.expires_from_now(boost::posix_time::seconds(10));
}

void Connection::close(Connection::CloseType t) {
    //if (is_finished_) {
    //    return;
    //}

    //is_finished_ = true;
    //auto fd1 = local_fwd_->socket()->native_handle();
    //auto fd2 = remote_fwd_->socket()->native_handle();
    //auto is1 = local_fwd_->socket()->is_open();
    //auto is2 = remote_fwd_->socket()->is_open();
    //LOG_ACPROXY_DEBUG(fd1, " ", fd2, " is closed");
    //LOG_ACPROXY_DEBUG("is_open = ", is1, ", ", is2);

    if ((t & Local) && local_fwd_ && local_fwd_->socket()->is_open()) {
        //local_fwd_->socket()->close();
        std::call_once(close_local_flag_,
                       [&]() { local_fwd_->socket()->close(); });
    }
    if ((t & Remote) && remote_fwd_ && remote_fwd_->socket()->is_open()) {
        //remote_fwd_->socket()->close();
        std::call_once(close_remote_flag_,
                       [&]() { remote_fwd_->socket()->close(); });
    }
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

void Connection::timeout() {
    // timeout occurs
    if (timeout_.expires_at() <=
        boost::asio::deadline_timer::traits_type::now()) {
        LOG_ACPROXY_INFO("connection timeout 10s, need to be closed");
        // stop this connection
        auto ptr = shared_from_this();
        conn_mgr_.stop(ptr); // FIXME
        LOG_ACPROXY_DEBUG("use_count = ", ptr.use_count());
        LOG_ACPROXY_INFO("connection stop success");
        timeout_.expires_at(boost::posix_time::pos_infin);
        return;
    }

    timeout_.async_wait(boost::bind(&Connection::timeout, this));
}
}
