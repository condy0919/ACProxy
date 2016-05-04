#include "server.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <functional>
#include <thread>
#include <iostream>

namespace ACProxy {
Server::Server(const std::string& addr, int port, std::size_t thread_pool_size)
    : thread_pool_size_(thread_pool_size),
      signals_(io_service_),
      acceptor_(io_service_) /*,
      new_connection_(io_service_),
      request_handler_(io_service_)*/ {
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
    signals_.async_wait(boost::bind(&Server::handleStop, this));  // FIXME

    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver::query query(
        addr, boost::lexical_cast<std::string>(port));
    std::cout << "start to resolve\n";
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    std::cout << "resolve ends\n";
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    startAccept();
}

void Server::run() {
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
    new_connection_ =
        std::make_shared<Connection>(io_service_, request_handler_);
    acceptor_.async_accept(new_connection_->socket(),
                           boost::bind(&Server::handleAccept, this,  // FIXME
                                       boost::asio::placeholders::error));
}

void Server::handleAccept(const boost::system::error_code& e) {
    if (!e) {
        new_connection_->start();
    }
    startAccept();
}

void Server::handleStop() {
    io_service_.stop();
}
}
