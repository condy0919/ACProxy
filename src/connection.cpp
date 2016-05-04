#include "log.hpp"
#include "common.hpp"
#include "connection.hpp"
#include <boost/bind.hpp>
#include <functional>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <vector>

namespace ACProxy {
Connection::Connection(boost::asio::io_service& io_service)
    : strand_(io_service), socket_(io_service) {}

boost::asio::ip::tcp::socket& Connection::socket() {
    return socket_;
}

void Connection::start() {
    boost::asio::async_read_until(
        socket_, buffer_, Common::MiscStrings::crlfcrlf,
        strand_.wrap(
            boost::bind(&Connection::handleHeaderRead, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
}

void Connection::handleHeaderRead(const boost::system::error_code& e,
                                  std::size_t bytes_transferred) {
    LOG_ACPROXY_INFO("starting read http request header...");
    if (e) {
        const int err = errno;
        LOG_ACPROXY_ERROR("read http request header error ", std::strerror(err));
        return;
    }

    LOG_ACPROXY_INFO("starting to parse http request header...");
    boost::asio::streambuf::const_buffers_type bufs = buffer_.data();
    std::string str(boost::asio::buffers_begin(bufs),
                    boost::asio::buffers_begin(bufs) + bytes_transferred);

    Http::HeaderGrammar<decltype(str)::iterator> http_grammar;
    bool res = phrase_parse(str.begin(), str.end(), http_grammar,
                            boost::spirit::qi::ascii::blank, request_);
    LOG_ACPROXY_INFO("the result of parsing http request header = ", res);

    // consume header
    auto iter = std::search(str.begin(), str.end(),
                            std::begin(Common::MiscStrings::crlfcrlf),
                            std::end(Common::MiscStrings::crlfcrlf));
    std::string::iterator header_last =
        iter + sizeof(Common::MiscStrings::crlfcrlf);
    std::size_t discard = std::distance(str.begin(), header_last);
    buffer_.consume(discard);


    bool still_need_read = false;
    // handle content body
    if (res && (request_.method == "POST" || request_.method == "PUT" ||
                request_.method == "CONNECT" || request_.method == "PATCH")) {
        std::size_t content_length = request_.getContentLength();
        auto content_start = header_last;
        if (std::distance(content_start, str.end()) < content_length) {
            still_need_read = true;
        }
        LOG_ACPROXY_INFO(request_.method, " has content body, still_need_read = ", still_need_read);
    } else if (!res) {
        still_need_read = true;
        LOG_ACPROXY_INFO("http request header incomplete, read again");
    }

    if (!still_need_read) {
        LOG_ACPROXY_INFO("the whole http request =");
        std::cout << request_;
        LOG_ACPROXY_INFO("http request ends");

        LOG_ACPROXY_INFO("http request header completes, starting forward");
        // handle forward

        // filter

        // cache

        // query source

    } else {
        // GUARANTEE NO SYNTAX ERROR IN HEADER
        // content body is missing
        boost::asio::async_read_until(
            socket_, buffer_, Common::MiscStrings::crlfcrlf,
            strand_.wrap(
                boost::bind(&Connection::handleBodyRead, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred)));
        LOG_ACPROXY_INFO("register callback of reading http request content body");
    }
}

void Connection::handleBodyRead(const boost::system::error_code& e,
                                std::size_t bytes_transferred) {
    if (e) {
        LOG_ACPROXY_ERROR("read http request content body error");
        return;
    }

    // content body read completely
    boost::asio::streambuf::const_buffers_type bufs = buffer_.data();
    std::string content(boost::asio::buffers_begin(bufs),
                        boost::asio::buffers_end(bufs));
    buffer_.consume(content.size());
    request_.content = content;

    std::ostringstream oss;
    oss << request_;
    std::string req = oss.str();

    LOG_ACPROXY_DEBUG("the whole http request =");
    std::cout << req;
    LOG_ACPROXY_DEBUG("request ends");

    LOG_ACPROXY_INFO("register callback of http request forward");
    boost::asio::async_write(
        socket_, boost::asio::buffer(req.data(), req.size()),
        strand_.wrap(
            boost::bind(&Connection::handleForward, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
}

void Connection::handleForward(const boost::system::error_code& e,
                               std::size_t bytes_transferred) {
    if (e) {
        LOG_ACPROXY_ERROR("forward http request error");
        return;
    }

    LOG_ACPROXY_INFO("starting to forward http request to source destination");
    boost::system::error_code ignored;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored);
}
}
