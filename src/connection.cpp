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
#include <atomic>

namespace ACProxy {

Connection::Connection(boost::asio::io_service& io_service,
                       ConnectionManager& mgr)
    : io_service_(io_service),
      conn_mgr_(mgr),
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

void Connection::init() {
    local_fwd_ = std::make_shared<LocalForwarder>(shared_from_this());
    remote_fwd_ = std::make_shared<RemoteForwarder>(shared_from_this());
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

void Connection::report(std::string metric,
                        std::vector<std::pair<std::string, std::string>> tags,
                        std::time_t tm) {
    static std::atomic<std::size_t> failures(0);
    std::ostringstream oss;

    oss << metric;
    for (auto&& tag : tags) {
        oss << "," << tag.first << "=" << tag.second;
    }
    oss << " value=" << ++failures << " " << tm << "000000000";

    std::string data = std::move(oss.str());

    boost::asio::ip::tcp::iostream stream("127.0.0.1", "8086");
    stream.expires_from_now(boost::posix_time::seconds(10));
    //stream.connect("127.0.0.1", "8086");
    stream << "POST /write?db=acproxy HTTP/1.1\r\n"
           << "Host: 127.0.0.1:8086\r\n"
           << "Accept: */*\r\n"
           << "Content-Length: " << data.size() << "\r\n"
           << "Content-Type: application/x-www-form-urlencoded\r\n"
           << "Connection: close\r\n"
           << "\r\n"
           << data;
    stream.flush();
    LOG_ACPROXY_DEBUG(stream.rdbuf());
    stream.close();

    LOG_ACPROXY_INFO("report to influxdb done");
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
