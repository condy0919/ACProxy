#ifndef ACPROXY_HTTP_RESPONSE_HPP_
#define ACPROXY_HTTP_RESPONSE_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/optional.hpp>
#include <ostream>
#include <string>
#include <map>

namespace Http {
struct Response {
    std::string http_version;
    std::string status_code;
    std::string description;
    std::map<std::string, std::string> headers;
    boost::optional<std::string> content;

    bool isKeepAlive() const;
    const std::string getContentType() const;
    const std::size_t getContentLength() const;
    const std::string getContent() const;

    friend std::ostream& operator<<(std::ostream& os, const Response& resp);
};

template <typename Iter,
          typename Skipper = boost::spirit::qi::ascii::blank_type>
struct ResponseHeaderGrammar
    : boost::spirit::qi::grammar<Iter, Response(), Skipper> {
    ResponseHeaderGrammar()
        : ResponseHeaderGrammar::base_type(http_header,
                                           "ResponseHeaderGrammar Grammar") {
        http_ver = "HTTP/" >> +boost::spirit::qi::char_("0-9.");
        status = +boost::spirit::qi::digit;
        desc = +boost::spirit::qi::graph;

        field_key = +boost::spirit::qi::char_("0-9a-zA-Z-");
        field_value = +~boost::spirit::qi::char_("\r\n");

        fields = *(field_key >> ':' >> field_value >>
                   boost::spirit::qi::lexeme["\r\n"]);

        http_header = http_ver >> status >> desc >>
                      boost::spirit::qi::lexeme["\r\n"] >> fields >>
                      boost::spirit::qi::lexeme["\r\n"];
    }

private:
    boost::spirit::qi::rule<Iter, std::map<std::string, std::string>(), Skipper> fields;
    boost::spirit::qi::rule<Iter, Response(), Skipper> http_header;

    // lexemes
    boost::spirit::qi::rule<Iter, std::string()> http_ver, status, desc;
    boost::spirit::qi::rule<Iter, std::string()> field_key, field_value;
};
}

BOOST_FUSION_ADAPT_STRUCT(Http::Response, http_version, status_code, description, headers)

#endif
