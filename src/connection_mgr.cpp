#include "connection_mgr.hpp"
#include "log.hpp"

namespace ACProxy {
ConnectionManager::ConnectionManager() {}

void ConnectionManager::start(std::shared_ptr<Connection> conn) {
    conn->start();
    conns_.insert(conn);
}

void ConnectionManager::stop(std::shared_ptr<Connection> conn) {
    //conn->stop();
    {
        std::lock_guard<std::mutex> guard(mtx);
        conns_.erase(conn); // FIXME how to stop a connection properly?
    }
}

void ConnectionManager::stopAll() {
    //for (auto&& conn : conns_) {
    //    conn->stop();
    //}
    {
        std::lock_guard<std::mutex> guard(mtx);
        conns_.clear();
    }
}
}
