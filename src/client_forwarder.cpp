#include "log.hpp"
#include "connection.hpp"
#include "client_forwarder.hpp"
#include "server_forwarder.hpp"
#include <boost/bind.hpp>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>

namespace ACProxy {

ClientForwarder::ClientForwarder(std::observer_ptr<Connection> conn)
    : strand_(conn->getIOService()),
      socket_(
          std::make_shared<boost::asio::ip::tcp::socket>(conn->getIOService())),
      conn_(conn) {
    //socket_.non_blocking(true); // XXX
}

std::shared_ptr<boost::asio::ip::tcp::socket> ClientForwarder::socket() {
    return socket_;
}

void ClientForwarder::send(std::string data) {
    boost::asio::async_write(
        *socket_, boost::asio::buffer(data.data(), data.size()),
        strand_.wrap(boost::bind(&ClientForwarder::sendHandle,
                                 shared_from_this(),
                                 boost::asio::placeholders::error)));
}

void ClientForwarder::sendHandle(const boost::system::error_code& e) {
    if (!e) {
        LOG_ACPROXY_INFO("send response to client success");
    } else {
        LOG_ACPROXY_ERROR("send response to client error ", e.message());
    }
    socket_->close();
}

void ClientForwarder::getHeaders() {
    socket_->async_read_some(
        boost::asio::null_buffers(),
        strand_.wrap(boost::bind(&ClientForwarder::getHeadersHandle,
                                 shared_from_this(),
                                 boost::asio::placeholders::error)));
}

void ClientForwarder::getHeadersHandle(const boost::system::error_code& e) {
    if (e) {
        LOG_ACPROXY_ERROR("get http request header error ", e.message());
        return;
    }
    
    LOG_ACPROXY_INFO("starting reading http request headers...");

    static char buf[BUFSIZ];

    auto fd = socket_->native_handle();
    ssize_t sz = ::recv(fd, buf, sizeof(buf), MSG_PEEK);
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
            return;
        }

        if (!request_.isConnectMethod()) {
            request_.setNoKeepAlive();
            // forward header to server
            conn_->getServerForwarder()->send(request_.toBuffer());
        }

        getBody();
    } else {
        // request header is not complete, read again
        LOG_ACPROXY_INFO("http request header not complete, read again");
        getHeaders();
    }
}

void ClientForwarder::getBody() {
    boost::asio::async_read(
        *socket_, buffer_, boost::asio::transfer_at_least(1),
        strand_.wrap(boost::bind(&ClientForwarder::getBodyHandle,
                                 shared_from_this(),
                                 boost::asio::placeholders::error)));
}

void ClientForwarder::getBodyHandle(const boost::system::error_code& e) {
    if (!e) {
        // TODO zero-copy
        boost::asio::streambuf::const_buffers_type bufs = buffer_.data();
        std::string str(boost::asio::buffers_begin(bufs),
                        boost::asio::buffers_end(bufs));
        buffer_.consume(str.size());
        conn_->getServerForwarder()->send(str);
        getBody();
    } else if (e != boost::asio::error::eof) {
        LOG_ACPROXY_ERROR("read http request content body error ", e.message());
    } else {
        LOG_ACPROXY_INFO("http request content body EOF");
    }
}
}
