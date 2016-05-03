#include "response_parser.hpp"
#include "response.hpp"

namespace Http {
ResponseParser::ResponseParser() : state_(status_start) {}

void ResponseParser::reset() {
    state_ = status_start;
}

ResponseParser::ResultType ResponseParser::consume(Response& resp, char input) {
    switch (state_) {
    case status_start:
        if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            state_ = http_version_h;
            return indeterminate;
        }
    case http_version_h:
        if (input == 'H') {
            state_ = http_version_t_1;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_t_1:
        if (input == 'T') {
            state_ = http_version_t_2;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_t_2:
        if (input == 'T') {
            state_ = http_version_p;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_p:
        if (input == 'P') {
            state_ = http_version_slash;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_slash:
        if (input == '/') {
            resp.http_version_major = 0;
            resp.http_version_minor = 0;
            state_ = http_version_major_start;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_major_start:
        if (is_digit(input)) {
            resp.http_version_major =
                resp.http_version_major * 10 + (input - '0');
            state_ = http_version_major;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_major:
        if (input == '.') {
            state_ = http_version_minor_start;
            return indeterminate;
        } else if (is_digit(input)) {
            resp.http_version_major =
                resp.http_version_major * 10 + (input - '0');
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_minor_start:
        if (is_digit(input)) {
            resp.http_version_minor =
                resp.http_version_minor * 10 + (input - '0');
            state_ = http_version_minor;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_minor:
        if (input == ' ') {
            resp.http_status_code = 0;
            state_ = http_status_code;
            return indeterminate;
        } else if (is_digit(input)) {
            resp.http_version_minor =
                resp.http_version_minor * 10 + (input - '0');
            return indeterminate;
        } else {
            return bad;
        }
    case http_status_code:
        if (is_digit(input)) {
            resp.http_status_code =
                resp.http_status_code * 10 + (input - '0');
            return indeterminate;
        } else if (input == ' ') {
            state_ = http_status_msg;
            return indeterminate;
        } else {
            return bad;
        }
    case http_status_msg:
        if (input == '\r') {
            state_ = expecting_newline_1;
            return indeterminate;
        } else if (is_char(input)) {
            resp.http_status_msg.push_back(input);
            return indeterminate;
        } else {
            return bad;
        }
    case expecting_newline_1:
        if (input == '\n') {
            state_ = header_line_start;
            return indeterminate;
        } else {
            return bad;
        }
    case header_line_start:
        if (input == '\r') {
            state_ = expecting_newline_3;
            return indeterminate;
        } else if (!resp.headers.empty() &&
                   (input == ' ' || input == '\t')) {
            state_ = header_lws;
            return indeterminate;
        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            resp.headers.push_back(Header());
            resp.headers.back().name.push_back(input);
            state_ = header_name;
            return indeterminate;
        }
    case header_lws:
        if (input == '\r') {
            state_ = expecting_newline_2;
            return indeterminate;
        } else if (input == ' ' || input == '\t') {
            return indeterminate;
        } else if (is_ctl(input)) {
            return bad;
        } else {
            state_ = header_value;
            resp.headers.back().value.push_back(input);
            return indeterminate;
        }
    case header_name:
        if (input == ':') {
            state_ = space_before_header_value;
            return indeterminate;
        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            resp.headers.back().name.push_back(input);
            return indeterminate;
        }
    case space_before_header_value:
        if (input == ' ') {
            state_ = header_value;
            return indeterminate;
        } else {
            return bad;
        }
    case header_value:
        if (input == '\r') {
            state_ = expecting_newline_2;
            return indeterminate;
        } else if (is_ctl(input)) {
            return bad;
        } else {
            resp.headers.back().value.push_back(input);
            return indeterminate;
        }
    case expecting_newline_2:
        if (input == '\n') {
            state_ = header_line_start;
            return indeterminate;
        } else {
            return bad;
        }
    case expecting_newline_3:
        return (input == '\n') ? good : bad;
    default:
        return bad;
    }
}

bool ResponseParser::is_char(int c) {
    return c >= 0 && c <= 127;
}

bool ResponseParser::is_ctl(int c) {
    return (c >= 0 && c <= 31) || (c == 127);
}

bool ResponseParser::is_tspecial(int c) {
    switch (c) {
    case '(':
    case ')':
    case '<':
    case '>':
    case '@':
    case ',':
    case ';':
    case ':':
    case '\\':
    case '"':
    case '/':
    case '[':
    case ']':
    case '?':
    case '=':
    case '{':
    case '}':
    case ' ':
    case '\t':
        return true;
    default:
        return false;
    }
}

bool ResponseParser::is_digit(int c) {
    return c >= '0' && c <= '9';
}
}
