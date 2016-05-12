#pragma once

#include "connection.hpp"
#include <boost/noncopyable.hpp>
#include <unordered_set>
#include <memory>
#include <mutex>
#include <set>

namespace ACProxy {

class ConnectionManager : private boost::noncopyable {
public:
    ConnectionManager();

    void start(std::shared_ptr<Connection> conn);

    void stop(std::shared_ptr<Connection> conn);

    void stopAll();

private:
    std::mutex mtx;

    //std::set<std::shared_ptr<Connection>> conns_;
    std::unordered_set<std::shared_ptr<Connection>> conns_;
};
}
