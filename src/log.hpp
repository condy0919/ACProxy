#ifndef ACPROXY_LOG_HPP_
#define ACPROXY_LOG_HPP_

#include <sstream>
#include <cstddef>
#include <utility>
#include <boost/noncopyable.hpp>

namespace ACProxy {

class Logger : public boost::noncopyable {
public:
    enum {
        FATAL = 0x01,
        ERROR = 0x02,
        WARNING = 0x04,
        INFO = 0x08,
        DEBUG = 0x10
    };

private:
    std::stringstream buffer;
    const unsigned long long mask;
    const char* file;
    std::size_t line;

public:
    template <typename... Ts>
    Logger(unsigned long long mask_, const char* file_, std::size_t line_, Ts&&... Args) noexcept 
        : mask(mask_), file(file_), line(line_) {
        logger_recursive(std::forward<Ts>(Args)...);
    }

    ~Logger() noexcept;

private:
    template <typename Head, typename... Tail>
    void logger_recursive(Head&& h, Tail&&... tail) {
        put(std::forward<Head>(h));
        logger_recursive(std::forward<Tail>(tail)...);
    }

    void logger_recursive() {}

    template <typename T>
    void put(const T& val) {
        buffer << val;
    }

    void put(bool val) {
        buffer << std::boolalpha << val;
    }

    void put(signed char val) {
        buffer << (int)val;
    }

    void put(unsigned char val) {
        buffer << (int)val;
    }
};
}

#define LOG_ACPROXY_FATAL(...) \
    LOG_ACPROXY(::ACProxy::Logger::FATAL, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ACPROXY_ERROR(...) \
    LOG_ACPROXY(::ACProxy::Logger::ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ACPROXY_WARNING(...) \
    LOG_ACPROXY(::ACProxy::Logger::WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ACPROXY_INFO(...) \
    LOG_ACPROXY(::ACProxy::Logger::INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ACPROXY_DEBUG(...) \
    LOG_ACPROXY(::ACProxy::Logger::DEBUG, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_ACPROXY(mask, ...) \
    do { \
        static_cast<void>(::ACProxy::Logger(mask, __VA_ARGS__)); \
    } while (false)

#endif
