#include "log.hpp"
#include <mutex>
#include <ctime>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <sys/syscall.h>

namespace {
struct ColorInfo {
    const char* text;
    char color;
};

ColorInfo infos[] = {
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
    const bool useColor = ::isatty(STDOUT_FILENO);
    const ColorInfo& info = infos[__builtin_ctz(mask)];
    std::string line;
    char tmp[128];
    unsigned len;

    line.reserve(255);

    len = sprintf(tmp, "%ld ", std::time(0));
    line.append(tmp, len);
    len = sprintf(tmp, "%d ", syscall(SYS_gettid));
    line.append(tmp, len);
    if (useColor) {
        len = sprintf(tmp, "\033[3%cm[%s]\033[0m %s:%d ", info.color, info.text, file, this->line);
        line.append(tmp, len);
    } else {
        len = sprintf(tmp, "[%s] %s:%d ", info.text, file, this->line);
        line.append(tmp, len);
    }

    char ch;
    while (buffer.get(ch)) {
        if (((unsigned char)ch + 1 <= 0x20) || (ch == 0x7f)) {
            ch = ' ';
        }
        line.push_back(ch);
    }
    line += '\n';

    std::lock_guard<std::mutex> lck(log_mutex);
    std::size_t written = 0;
    while (written < line.size()) {
        ::ssize_t ret = ::write(STDOUT_FILENO, line.data() + written, line.size() - written);
        if (ret <= 0)
            break;
        written += ret;
    }
}
}
