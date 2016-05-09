#include "http/response.hpp"
#include <iostream>

int main() {
    std::string s = "HTTP/1.1 200 OK\r\n"
                    "Server: JSP3/2.0.14\r\n"
                    "Date: Mon, 09 May 2016 12:05:13 GMT\r\n"
                    "Content-Type: image/jpeg\r\n"
                    "Content-Length: 3857\r\n"
                    "Connection: close\r\n"
                    "Last-Modified: Sun, 03 Apr 2016 13:46:34 GMT \r\n"
                    "Expires: Sun, 02 Oct 2016 13:46:34 GMT \r\n"
                    "Age: 2931519\r\n"
                    "Accept-Ranges: bytes\r\n"
                    "Portrait_tag: f423b6b19fb53ca9faec1e9297ca21e7\r\n"
                    "\r\n";

    Http::Response response;
    Http::ResponseHeaderGrammar<decltype(s)::iterator> grammar;
    bool res = phrase_parse(s.begin(), s.end(), grammar,
                            boost::spirit::qi::ascii::blank, response);
    std::cout << res << std::endl;
}
