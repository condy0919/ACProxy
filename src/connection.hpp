#pragma once

#include "http/request.hpp"
#include <boost/asio.hpp>
#include <memory>

namespace ACProxy {

class LocalForwarder;
class RemoteForwarder;
class Connection : public std::enable_shared_from_this<Connection>,
                   private boost::noncopyable {
public:
    explicit Connection(boost::asio::io_service& io_service);
    ~Connection() noexcept;

    boost::asio::ip::tcp::socket& socket();

    void start();

    enum CloseType { Local = 0x1, Remote = 0x2, Both = Local | Remote };
    void close(CloseType t = Both);

    boost::asio::io_service& getIOService();

    std::shared_ptr<RemoteForwarder> getRemoteForwarder();

    std::shared_ptr<LocalForwarder> getLocalForwarder();

private:
    boost::asio::io_service& io_service_;
    std::shared_ptr<LocalForwarder> local_fwd_;
    std::shared_ptr<RemoteForwarder> remote_fwd_;
};
}
