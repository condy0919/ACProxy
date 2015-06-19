#include "log.hpp"
#include <mutex>
#include <ctime>
#include <string>
#include <chrono>
#include <cstdio>
#include <unistd.h>

namespace {
struct colorful_info {
    const char* text;
    char color;
};

colorful_info infos[] = {
    {"FATAL", '1'},    // Red
    {"ERROR", '5'},    // Pink
    {"WARNING", '3'},  // Yellow
    {"INFO", '2'},     // Green
    {"DEBUG", '7'}     // White
};

std::mutex log_mutex;
}

namespace ACProxy {
Logger::~Logger() noexcept {
    const static auto GetCurrentTime = []() -> std::string {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::string ret = std::asctime(std::localtime(&now_c));
        ret.pop_back();
        return ret;
    };

    const bool useColor = ::isatty(STDOUT_FILENO);
    const colorful_info& info = infos[__builtin_ctz(mask)];
    std::string line;
    char tmp[128];

    line += GetCurrentTime();
    line += " | ";
    if (useColor) {
        sprintf(tmp, "\033[3%cm[%s]\033[0m ", info.color, info.text);
    } else {
        sprintf(tmp, "[%s] ", info.text);
    }
    line += tmp;
    line += buffer.str();
    line += '\n';

    std::lock_guard<std::mutex> lcck(log_mutex);
    std::size_t written = 0;
    while (written < line.size()) {
        ::ssize_t ret = ::write(STDOUT_FILENO, line.data() + written, line.size() - written);
        if (ret <= 0)
            break;
        written += ret;
    }
}
}
