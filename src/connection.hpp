#pragma once

#include "http/request.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <atomic>
#include <mutex>

namespace ACProxy {

class LocalForwarder;
class RemoteForwarder;
class ConnectionManager;
class Connection : public std::enable_shared_from_this<Connection>,
                   private boost::noncopyable {
public:
    explicit Connection(boost::asio::io_service& io_service,
                        ConnectionManager& mgr);

    ~Connection() noexcept;

    boost::asio::ip::tcp::socket& socket();

    void start();

    void stop(); // an alias of close

    void update(); // update at the end of handle

    enum CloseType { Local = 0x1, Remote = 0x2, Both = Local | Remote };
    void close(CloseType t = Both);

    boost::asio::io_service& getIOService();

    std::shared_ptr<RemoteForwarder> getRemoteForwarder();

    std::shared_ptr<LocalForwarder> getLocalForwarder();

private:
    void timeout();

private:
    boost::asio::io_service& io_service_;
    ConnectionManager& conn_mgr_;
    std::shared_ptr<LocalForwarder> local_fwd_;
    std::shared_ptr<RemoteForwarder> remote_fwd_;
    std::once_flag close_local_flag_, close_remote_flag_;
    //std::atomic<bool> is_finished_;

    boost::asio::deadline_timer timeout_;
};
}
