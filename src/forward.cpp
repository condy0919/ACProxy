#include "common.hpp"
#include "forward.hpp"
#include "connection.hpp"
#include "http/response.hpp"
#include <boost/bind.hpp>
#include <sstream>

namespace ACProxy {
Forwarder::Forwarder(std::shared_ptr<Connection> conn, Http::Request req)
    : conn_(conn),
      service_(conn->socket().get_io_service()),
      sock_(service_),
      strand_(service_),
      request_(req) {
    //request_.setNoKeepAlive();
}

void Forwarder::start() {
    const std::string host = request_.getHost();
    const std::string port =
        boost::lexical_cast<std::string>(request_.getPort());

    LOG_ACPROXY_DEBUG("host = ", host, " port = ", port);

    boost::asio::ip::tcp::resolver resolver(service_);
    boost::asio::ip::tcp::resolver::query query(host, port);
    boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
    boost::asio::ip::tcp::endpoint ep = *iter;
    LOG_ACPROXY_INFO("connect to ", host, ":", port);

    sock_.open(boost::asio::ip::tcp::v4());
    sock_.connect(ep);

    // start to send data
    std::ostringstream oss;
    oss << request_;
    std::string str = oss.str();

    boost::asio::async_write(
        sock_, boost::asio::buffer(str.data(), str.size()),
        strand_.wrap(
            boost::bind(&Forwarder::handleRequestWrite, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
}

void Forwarder::handleRequestWrite(const boost::system::error_code& e,
                                   std::size_t bytes_transferred) {
    if (e) {
        LOG_ACPROXY_ERROR("forward http request error, ", e.message());
        return;
    }

    // success, register handle of reading http response header
    LOG_ACPROXY_INFO("forward http request success");
    boost::asio::async_read_until(
        sock_, buffer_, Common::MiscStrings::crlfcrlf,
        strand_.wrap(
            boost::bind(&Forwarder::handleHeaderRead, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
}

void Forwarder::handleHeaderRead(const boost::system::error_code& e,
                                 std::size_t bytes_transferred) {
    if (e) {
        LOG_ACPROXY_ERROR("read http response header error, ", e.message());
        return;
    }

    // success, start to parse http response header
    // according to Content-Length, need to read response body
    LOG_ACPROXY_INFO("read http response header success");
    LOG_ACPROXY_INFO("starting to parse http response header...");
    boost::asio::streambuf::const_buffers_type bufs = buffer_.data();
    std::string str(boost::asio::buffers_begin(bufs),
                    boost::asio::buffers_begin(bufs) + bytes_transferred);

    Http::ResponseHeaderGrammar<decltype(str)::iterator> http_grammar;
    bool res = phrase_parse(str.begin(), str.end(), http_grammar,
                            boost::spirit::qi::ascii::blank, response_);
    LOG_ACPROXY_INFO("the result of parsing http response header = ", res);
    if (!res) {
        LOG_ACPROXY_ERROR("http response parse error");
        return;
    }

    // consume header
    auto iter = std::search(str.begin(), str.end(),
                            std::begin(Common::MiscStrings::crlfcrlf),
                            std::end(Common::MiscStrings::crlfcrlf));
    std::string::iterator header_last =
        iter + sizeof(Common::MiscStrings::crlfcrlf);
    std::size_t discard = std::distance(str.begin(), header_last);
    buffer_.consume(discard);

    content_body_remain = response_.getContentLength();

    bool still_need_read = (request_.method != "HEAD");
    LOG_ACPROXY_DEBUG("the whole http response = ");
    std::cout << response_;
    LOG_ACPROXY_DEBUG("http response ends");
    forwardHeaderToClient();
    if (!still_need_read) {
        LOG_ACPROXY_INFO(
            "http response header completes, starting forward to client");
    } else {
        // content body is missing
        LOG_ACPROXY_DEBUG("http response content body");
        boost::asio::async_read(
            sock_, buffer_, boost::asio::transfer_at_least(1),
            strand_.wrap(
                boost::bind(&Forwarder::handleBodyRead, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred)));
        LOG_ACPROXY_INFO(
            "register callback of reading http response content body");
    }
}

void Forwarder::handleBodyRead(const boost::system::error_code& e,
                               std::size_t bytes_transferred) {
    if (e) {
        LOG_ACPROXY_ERROR("read http response body error, ", e.message());
        return;
    }

    //content_body_remain -= bytes_transferred;
    // forward data to client
    forwardBodyToClient();

    if (content_body_remain > 0) {
        // register again
        boost::asio::async_read(
            sock_, buffer_, boost::asio::transfer_at_least(1),
            strand_.wrap(
                boost::bind(&Forwarder::handleBodyRead, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred)));
    }
}

void Forwarder::forwardBodyToClient() {
    LOG_ACPROXY_INFO("register send response body callback of quirier");
    boost::asio::streambuf::const_buffers_type bufs = buffer_.data();
    std::string content(boost::asio::buffers_begin(bufs),
                        boost::asio::buffers_end(bufs));
    buffer_.consume(content.size());

    content_body_remain -= content.size();

    conn_->reply(content);
}

void Forwarder::forwardHeaderToClient() {
    LOG_ACPROXY_INFO("register send response header callback of quirier");
    std::ostringstream oss;
    oss << response_;
    std::string data = oss.str();
    conn_->reply(data);
}
}
