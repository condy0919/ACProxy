#include "log.hpp"

int main() {
    LOG_ACPROXY_FATAL("test");
    LOG_ACPROXY_ERROR("test");
    LOG_ACPROXY_WARNING("test");
    LOG_ACPROXY_INFO("test");
    LOG_ACPROXY_DEBUG("test");
    return 0;
}
