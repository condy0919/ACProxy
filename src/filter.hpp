#pragma once

#include <boost/noncopyable.hpp>
#include <vector>
#include <string>


namespace ACProxy {

class Filter : private boost::noncopyable {
public:
    Filter(std::string file);

    void add(std::string s);

    bool check(std::string s);

private:
    std::vector<std::string> black_list_;
};
}
