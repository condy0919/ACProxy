#include "log.hpp"
#include "connection.hpp"
#include "local_fwd.hpp"
#include "remote_fwd.hpp"
#include "singleton/global.hpp"
#include <boost/bind.hpp>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>

namespace ACProxy {

LocalForwarder::LocalForwarder(std::shared_ptr<Connection> conn)
    : socket_(
          std::make_shared<boost::asio::ip::tcp::socket>(conn->getIOService())),
      conn_(conn) {}

LocalForwarder::~LocalForwarder() noexcept {
    stop();
    LOG_ACPROXY_INFO("LocalForwarder is freed...");
}

std::shared_ptr<boost::asio::ip::tcp::socket> LocalForwarder::socket() {
    return socket_;
}

void LocalForwarder::start() {
    getHeaders();
}

void LocalForwarder::stop() {
    if (socket_->is_open()) {
        std::call_once(close_flag_, [&]() { socket_->close(); });
    }
}

void LocalForwarder::send(std::string data) {
    boost::asio::async_write(
        *socket_, boost::asio::buffer(data.data(), data.size()),
        boost::bind(&LocalForwarder::sendHandle, shared_from_this(),
                    boost::asio::placeholders::error));
}

void LocalForwarder::sendHandle(const boost::system::error_code& e) {
    if (!e) {
        if (auto c = conn_.lock()) {
            c->update();
            LOG_ACPROXY_INFO("send response to client success");
        } else {
            LOG_ACPROXY_WARNING("connection object is freed");
        }
    } else {
        LOG_ACPROXY_ERROR("send response to client error ", e.message());
        //conn_->close();
    }
}

void LocalForwarder::finish(std::string data) {
    boost::asio::async_write(
        *socket_, boost::asio::buffer(data.data(), data.size()),
        boost::bind(&LocalForwarder::finishHandle, shared_from_this(),
                    boost::asio::placeholders::error));
}

void LocalForwarder::finishHandle(const boost::system::error_code& e) {
    if (!e) {
        if (auto c = conn_.lock()) {
            c->stop();
            LOG_ACPROXY_INFO("finish connection");
        } else {
            LOG_ACPROXY_WARNING("connection object is freed");
        }
    } else {
        if (auto c = conn_.lock()) {
            c->stop();
        } else {
            LOG_ACPROXY_WARNING("connection object is freed");
        }
        LOG_ACPROXY_ERROR("finish connection error ", e.message());
    }
}

void LocalForwarder::getHeaders() {
    socket_->async_read_some(
        boost::asio::null_buffers(),
        boost::bind(&LocalForwarder::getHeadersHandle, shared_from_this(),
                    boost::asio::placeholders::error));
}

void LocalForwarder::getHeadersHandle(const boost::system::error_code& e) {
    if (e) {
        LOG_ACPROXY_ERROR("get http request header error ", e.message());
        //conn_->close();
        return;
    }

    LOG_ACPROXY_INFO("start reading http request headers...");

    char buf[1024];  // XXX

    auto fd = socket_->native_handle();
    ssize_t sz = ::recv(fd, buf, sizeof(buf), MSG_PEEK);
    LOG_ACPROXY_DEBUG("sz of read http request header = ", sz);
    if (sz < 0) {
        const int err = errno;
        if (err == EAGAIN || err == EWOULDBLOCK) {
            LOG_ACPROXY_INFO("read http request header again...");
            getHeaders();
            return;
        }
        LOG_ACPROXY_INFO("read http request header error");
        //conn_->close();
        return;
    } else if (sz == 0) {
        LOG_ACPROXY_INFO("client close socket");
        //conn_->close();
        return;
    }
    const char* pos = (const char*)::memmem(buf, sz, "\r\n\r\n", 4);

    bool found = pos;
    ssize_t diff = (found ? pos - buf + 4 : sz);
    if (!found) {
        // \r | \n\r\n
        if (headers.size() >= 1 && headers.back() == '\r' && sz >= 3 &&
            buf[0] == '\n' && buf[1] == '\r' && buf[2] == '\n') {
            found = true;
            diff = 3;
        }

        // \r\n | \r\n
        if (headers.size() >= 2 && headers[headers.size() - 2] == '\r' &&
            headers[headers.size() - 1] == '\n' && sz >= 2 && buf[0] == '\r' &&
            buf[1] == '\n') {
            found = true;
            diff = 2;
        }

        // \r\n\r | \n
        if (headers.size() >= 3 && headers[headers.size() - 3] == '\r' &&
            headers[headers.size() - 2] == '\n' &&
            headers[headers.size() - 1] == '\r' && sz >= 1 && buf[0] == '\n') {
            found = true;
            diff = 1;
        }
    }

    // consume
    ::recv(fd, buf, diff, 0);
    headers.insert(headers.end(), buf, buf + diff);
    if (found) {
        Http::RequestHeaderGrammar<decltype(headers)::iterator> grammar;
        bool res = phrase_parse(headers.begin(), headers.end(), grammar,
                                boost::spirit::qi::ascii::blank, request_);
        if (!res) {
            LOG_ACPROXY_ERROR("parse http request header error");
            //conn_->close();
            return;
        }

        request_.rewrite();
        request_.setKeepAlive(false);

        // cache layer
        if (request_.method == "GET" || request_.method == "HEAD") {
            std::string key = keygen(request_.getHost(), request_.getPort(),
                                     request_.uri, request_.method);
            auto& cache = getGlobalCache();
            if (boost::optional<std::string> resp = cache.get(key)) {
                LOG_ACPROXY_INFO("hit, found in cache!");
                finish(resp.value());
                return;
            }
            if (auto c = conn_.lock()) {
                c->getRemoteForwarder()->setCacheKey(key);
            } else {
                LOG_ACPROXY_WARNING("connection object is freed");
                return;
            }
        }

        // LOG_ACPROXY_DEBUG("request = \n", request_.toBuffer());

        if (!request_.hasResponseBody()) {
            if (auto c = conn_.lock()) {
                c->getRemoteForwarder()->setResponseBody(false);
            } else {
                LOG_ACPROXY_WARNING("connection object is freed");
                return;
            }
        }

        std::string host = request_.getHost();
        int port = request_.getPort();
        if (auto c = conn_.lock()) {
            res = c->getRemoteForwarder()->connect(host, port);
        } else {
            LOG_ACPROXY_WARNING("connection object is freed");
            return;
        }
        if (!res) {
            if (auto c = conn_.lock()) {
                c->report("failure", {{"host", boost::asio::ip::host_name()}},
                          std::time(0));
            }
            // XXX Maybe shutdowning socket is better
            LOG_ACPROXY_INFO("connection timeout, close it");
            //send("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
            //conn_->close();
            return;
        }
        LOG_ACPROXY_DEBUG("connection complete");

        if (!request_.isConnectMethod()) {
            // forward header to server
            if (auto c = conn_.lock()) {
                c->getRemoteForwarder()->send(request_.toBuffer());
            } else {
                LOG_ACPROXY_WARNING("connection object is freed");
                return;
            }
        } else {
            // no parse response header, get rawdata and forward instead
            if (auto c = conn_.lock()) {
                c->getRemoteForwarder()->setParseResponseHeader(false);
            } else {
                LOG_ACPROXY_WARNING("connection object is freed");
                return;
            }
            // send a fake 200 response in tunnel mode
            send("HTTP/1.1 200 Connection Established\r\n\r\n");
        }

        if (request_.hasContentBody()) {
            getBody();
        }
    } else {
        // request header is incomplete, read again
        LOG_ACPROXY_INFO("http request header incomplete, read again");
        getHeaders();
    }
    if (auto c = conn_.lock()) {
        c->update();
    } else {
        LOG_ACPROXY_WARNING("connection object is freed");
        return;
    }
}

void LocalForwarder::getBody() {
    boost::asio::async_read(
        *socket_, buffer_, boost::asio::transfer_at_least(1),
        boost::bind(&LocalForwarder::getBodyHandle, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void LocalForwarder::getBodyHandle(const boost::system::error_code& e,
                                   std::size_t bytes_transferred) {
    if (bytes_transferred == 0) {
        LOG_ACPROXY_INFO("no http request body...");
        //conn_->close();
        return;
    }
    if (!e) {
        LOG_ACPROXY_INFO("start to read http request body...");

        // TODO zero-copy
        boost::asio::streambuf::const_buffers_type bufs = buffer_.data();
        std::string str(boost::asio::buffers_begin(bufs),
                        boost::asio::buffers_end(bufs));
        buffer_.consume(str.size());
        if (auto c = conn_.lock()) {
            c->getRemoteForwarder()->send(str);
            getBody();
            c->update();
        } else {
            LOG_ACPROXY_WARNING("connection object is freed");
        }
    } else if (e != boost::asio::error::eof &&
               e != boost::asio::error::connection_reset) {
        LOG_ACPROXY_ERROR("read http request content body error ", e.message());
        //conn_->close();
    } else {
        LOG_ACPROXY_INFO("http request content body EOF");
        //conn_->close();
    }
}
}
