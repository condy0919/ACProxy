#ifndef HTTP_REQUEST_HPP_
#define HTTP_REQUEST_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/optional.hpp>
#include <ostream>
#include <utility>
#include <vector>
#include <string>

namespace Http {

struct Request {
    std::string method;
    std::string uri;
    std::string http_version;
    std::vector<std::pair<std::string, std::string>> headers;
    //std::map<std::string, std::string> headers;
    boost::optional<std::string> content;

    void setKeepAlive(bool on = true);
    bool isKeepAlive() const;
    bool hasContentBody() const;
    bool hasResponseBody() const;
    const std::string getHost() const;
    const int getPort() const;
    std::size_t getContentLength() const;
    void rewrite();
    std::string toBuffer() const;
    bool isConnectMethod() const;

    friend std::ostream& operator<<(std::ostream& os, const Request& req);

private:
    void setHeader(std::string name, std::string value);
    const std::string getHeader(std::string name) const;
};

template <typename Iter,
          typename Skipper = boost::spirit::qi::ascii::blank_type>
struct RequestHeaderGrammar
    : boost::spirit::qi::grammar<Iter, Request(), Skipper> {
    RequestHeaderGrammar()
        : RequestHeaderGrammar::base_type(http_header,
                                          "RequestHeaderGrammar Grammar") {
        method = +boost::spirit::qi::alpha;
        uri = +boost::spirit::qi::graph;
        http_ver = "HTTP/" >> +boost::spirit::qi::char_("0-9.");

        field_key = +boost::spirit::qi::char_("0-9a-zA-Z-");
        field_value = +~boost::spirit::qi::char_("\r\n");

        fields = *(field_key >> ':' >> field_value >>
                   boost::spirit::qi::lexeme["\r\n"]);

        http_header = method >> uri >> http_ver >>
                      boost::spirit::qi::lexeme["\r\n"] >> fields >>
                      boost::spirit::qi::lexeme["\r\n"];
    }

private:
    boost::spirit::qi::rule<Iter, std::vector<std::pair<std::string, std::string>>(), Skipper> fields;
    boost::spirit::qi::rule<Iter, Request(), Skipper> http_header;

    // lexemes
    boost::spirit::qi::rule<Iter, std::string()> method, uri, http_ver;
    boost::spirit::qi::rule<Iter, std::string()> field_key, field_value;
};
}

BOOST_FUSION_ADAPT_STRUCT(Http::Request, method, uri, http_version, headers)

#endif
