#include "connection_mgr.hpp"

namespace ACProxy {
ConnectionManager::ConnectionManager() {}

void ConnectionManager::start(std::shared_ptr<Connection> conn) {
    conn->start();
    conns_.insert(conn);
}

void ConnectionManager::stop(std::shared_ptr<Connection> conn) {
    conn->stop();
    conns_.erase(conn);
}

void ConnectionManager::stopAll() {
    for (auto&& conn : conns_) {
        conn->stop();
    }
    conns_.clear();
}
}
