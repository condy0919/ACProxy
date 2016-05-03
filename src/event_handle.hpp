#ifndef ACPROXY_EVENT_HANDLE_HPP_
#define ACPROXY_EVENT_HANDLE_HPP_

#include <string>

namespace ACProxy {

class EventHandle {
public:
    EventHandle() = default;
    virtual ~EventHandle() noexcept {}

    virtual int getHandle() const = 0;
    virtual int onRead() {}
    virtual int onWrite() {}
    virtual int onError() {}

    virtual std::string getName() const {
        return "EventHandle";
    }
};
}

#endif
