#ifndef HTTP_RESPONSE_PARSER_HPP_
#define HTTP_RESPONSE_PARSER_HPP_

#include <tuple>

namespace Http {
struct Response;

class ResponseParser {
public:
    enum ResultType { good, bad, indeterminate };

    ResponseParser();

    void reset();

    template <typename InputIterator>
    std::tuple<ResultType, InputIterator> parse(Response& resp,
                                                InputIterator begin,
                                                InputIterator end) {
        while (begin != end) {
            ResultType res = consume(resp, *begin++);
            if (res == good || res == bad) {
                return std::make_tuple(res, begin);
            }
        }
        return std::make_tuple(indeterminate, begin);
    }

private:
    ResultType consume(Response& resp, char input);

    static bool is_char(int c);
    static bool is_ctl(int c);
    static bool is_tspecial(int c);
    static bool is_digit(int c);

    enum state {
        status_start,
        http_version_h,
        http_version_t_1,
        http_version_t_2,
        http_version_p,
        http_version_slash,
        http_version_major_start,
        http_version_major,
        http_version_minor_start,
        http_version_minor,
        http_status_code,
        http_status_msg,
        expecting_newline_1,
        header_line_start,
        header_lws,
        header_name,
        space_before_header_value,
        header_value,
        expecting_newline_2,
        expecting_newline_3
    } state_;
};
}

#endif
