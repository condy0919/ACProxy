#include "log.hpp"
#include "server.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <functional>
#include <thread>

namespace ACProxy {
Server::Server(const std::string& addr, int port, std::size_t thread_pool_size)
    : thread_pool_size_(thread_pool_size),
      signals_(io_service_),
      acceptor_(io_service_) {
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
    signals_.async_wait(boost::bind(&Server::handleStop, this));  // FIXME
    LOG_ACPROXY_INFO("register handleStop...");

    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver::query query(
        addr, boost::lexical_cast<std::string>(port));
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    LOG_ACPROXY_INFO("listen socket");

    startAccept();
}

void Server::run() {
    LOG_ACPROXY_INFO("starting server...");
    std::vector<std::thread> threads;
    for (std::size_t i = 0; i < thread_pool_size_; ++i) {
        threads.emplace_back(
            boost::bind(&boost::asio::io_service::run, &io_service_));
    }
    std::for_each(threads.begin(), threads.end(), [](auto&& t) {
        if (t.joinable())
            t.join();
    });
}

void Server::startAccept() {
    LOG_ACPROXY_INFO("register ACCEPT event");

    // TODO eliminate connection obj
    new_connection_ =
        std::make_shared<Connection>(io_service_, conn_mgr_);

    acceptor_.async_accept(new_connection_->socket(),
                           boost::bind(&Server::handleAccept, this,  // FIXME
                                       boost::asio::placeholders::error));
}

void Server::handleAccept(const boost::system::error_code& e) {
    if (!e) {
        LOG_ACPROXY_INFO("new connection established");
        conn_mgr_.start(new_connection_);
    }
    startAccept();
}

void Server::handleStop() {
    LOG_ACPROXY_INFO("stopping server...");
    acceptor_.close();
    conn_mgr_.stopAll();
    io_service_.stop();
}
}
