#include "log.hpp"
#include "server.hpp"
#include <thread>

int main(int argc, char* argv[]) {
    // TODO parse opt
    
    // TODO catch exception
    std::size_t num_threads = std::thread::hardware_concurrency();
    LOG_ACPROXY_INFO("starting with ", num_threads, " threads, binding to 127.0.0.1:8086");
    num_threads = 1;
    ACProxy::Server serv("127.0.0.1", 8086, num_threads);
    serv.run();

    return 0;
}
