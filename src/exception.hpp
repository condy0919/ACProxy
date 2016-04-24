#ifndef ACPROXY_EXCEPTION_HPP_
#define ACPROXY_EXCEPTION_HPP_

#include "log.hpp"
#include <exception>

namespace ACProxy {

class SystemException : public std::exception {
public:
    const char* what() const noexcept override {
        return "System Error";
    }
};
}

#define DEBUG_THROW_SYSTEMEXCEPTION(...) \
    do { \
        LOG_ACPROXY_DEBUG(__VA_ARGS__); \
        throw ACProxy::SystemException(); \
    } while (0)

#endif
