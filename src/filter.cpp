#include "filter.hpp"
#include <regex>

namespace ACProxy {

void Filter::add(std::string s) {
    black_list_.push_back(std::move(s));
}

bool Filter::check(std::string s) {
    std::smatch match;

    for (auto pattern : black_list_) {
        std::regex p(std::move(pattern));
        if (std::regex_match(s, match, p) && match.ready()) {
            return true;
        }
    }
    return false;
}
}
