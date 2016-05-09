#include "log.hpp"
#include "http/response.hpp"
#include "remote_fwd.hpp"
#include "local_fwd.hpp"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/types.h>
#include <sys/socket.h>

namespace ACProxy {

RemoteForwarder::RemoteForwarder(std::observer_ptr<Connection> conn)
    : strand_(conn->getIOService()),
      socket_(
          std::make_shared<boost::asio::ip::tcp::socket>(conn->getIOService())),
      conn_(conn) {
    ;  //
}

RemoteForwarder::~RemoteForwarder() noexcept {
    LOG_ACPROXY_INFO("RemoteForwarder is freed...");
}

std::shared_ptr<boost::asio::ip::tcp::socket> RemoteForwarder::socket() {
    return socket_;
}

void RemoteForwarder::socket(std::shared_ptr<boost::asio::ip::tcp::socket> sock) {
    socket_ = sock;
}

void RemoteForwarder::connect(std::string host, int port) {
    LOG_ACPROXY_DEBUG("host = ", host, " port = ", port);

    boost::asio::ip::tcp::resolver resolver(conn_->getIOService());
    boost::asio::ip::tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
    boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
    boost::asio::ip::tcp::endpoint ep = *iter;

    socket_->open(boost::asio::ip::tcp::v4());
    socket_->connect(ep);
}

void RemoteForwarder::setResponseBody(bool on) noexcept {
    has_response_body_ = on;
}

void RemoteForwarder::setParseResponseHeader(bool on) noexcept {
    parse_response_header_ = on;
}

void RemoteForwarder::send(std::string data) {
    boost::asio::async_write(
        *socket_, boost::asio::buffer(data.data(), data.size()),
        strand_.wrap(boost::bind(&RemoteForwarder::sendHandle,
                                 shared_from_this(),
                                 boost::asio::placeholders::error)));
}

void RemoteForwarder::sendHandle(const boost::system::error_code& e) {
    if (!e) {
        LOG_ACPROXY_INFO(
            "send data to upstream success, starting recving http response...");
        if (get_header_once_) {
            if (parse_response_header_) {
                getHeaders();
            } else {
                getRawData();
            }
            get_header_once_ = false;
        }
    } else {
        LOG_ACPROXY_ERROR("send data to upstream error ", e.message());
    }
}

void RemoteForwarder::getRawData() {
    boost::asio::async_read(
        *socket_, boost::asio::buffer(raw_data_),
        boost::asio::transfer_at_least(1),
        strand_.wrap(
            boost::bind(&RemoteForwarder::getRawDataHandle, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
}

void RemoteForwarder::getRawDataHandle(const boost::system::error_code& e,
                                       std::size_t bytes_transferred) {
    if (bytes_transferred == 0) {
        LOG_ACPROXY_INFO("no response data...");
        return;
    }
    if (!e) {
        conn_->getLocalForwarder()->send(std::string(
            raw_data_.data(), raw_data_.data() + bytes_transferred));
        getRawData();
    } else if (e != boost::asio::error::eof) {
        LOG_ACPROXY_ERROR("read raw data error ", e.message());
    } else {
        LOG_ACPROXY_INFO("read raw data EOF");
    }
}

void RemoteForwarder::getHeaders() {
    socket_->async_read_some(
        boost::asio::null_buffers(),
        strand_.wrap(boost::bind(&RemoteForwarder::getHeadersHandle,
                                 shared_from_this(),
                                 boost::asio::placeholders::error)));
}

void RemoteForwarder::getHeadersHandle(const boost::system::error_code& e) {
    if (e) {
        LOG_ACPROXY_ERROR("get http response header error ", e.message());
        return;
    }

    LOG_ACPROXY_INFO("starting reading http response headers...");

    char buf[1024]; // XXX

    auto fd = socket_->native_handle();
    ssize_t sz = ::recv(fd, buf, sizeof(buf), MSG_PEEK);
    if (sz < 0) {
        LOG_ACPROXY_ERROR("peek http response header error");
        return;
    }
    //LOG_ACPROXY_DEBUG("remote buf = \n", buf);
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
        Http::ResponseHeaderGrammar<decltype(headers)::iterator> grammar;
        bool res = phrase_parse(headers.begin(), headers.end(), grammar,
                                boost::spirit::qi::ascii::blank, response_);

        //LOG_ACPROXY_DEBUG("response = \n", response_.toBuffer());

        if (!res) {
            LOG_ACPROXY_ERROR("parse http response header error");
            return;
        }

        response_.setNoKeepAlive();
        conn_->getLocalForwarder()->send(response_.toBuffer());

        if (has_response_body_) {
            getBody();
        }
    } else {
        LOG_ACPROXY_INFO("http response header not complete, read again");
        getHeaders();
    }
}

void RemoteForwarder::getBody() {
    boost::asio::async_read(
        *socket_, buffer_, boost::asio::transfer_at_least(1),
        strand_.wrap(
            boost::bind(&RemoteForwarder::getBodyHandle, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
}

void RemoteForwarder::getBodyHandle(const boost::system::error_code& e,
                                    std::size_t bytes_transferred) {
    if (bytes_transferred == 0) {
        LOG_ACPROXY_INFO("no http response body...");
        return;
    }

    if (!e) {
        // TODO zero-copy
        boost::asio::streambuf::const_buffers_type bufs = buffer_.data();
        std::string str(boost::asio::buffers_begin(bufs),
                        boost::asio::buffers_end(bufs));
        buffer_.consume(str.size());
        LOG_ACPROXY_DEBUG("send ", bytes_transferred, " bytes to LocalForwarder");
        conn_->getLocalForwarder()->send(str);
        getBody();
        //socket_->close(); // TODO socket pool, no need to close
    } else if (e != boost::asio::error::eof) {
        LOG_ACPROXY_ERROR("read http response content body error ", e.message());
    } else {
        LOG_ACPROXY_INFO("http response content body EOF");
    }
}
}
