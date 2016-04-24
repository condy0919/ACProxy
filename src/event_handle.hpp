#ifndef ACPROXY_EVENT_HANDLE_HPP_
#define ACPROXY_EVENT_HANDLE_HPP_

namespace ACProxy {

class EventHandle {
public:
    EventHandle() = default;
    virtual ~EventHandle() noexcept {}

    virtual int getHandle() const = 0;
    virtual void onRead() {}
    virtual void onWrite() {}
    virtual void onError() {}
};
}

#endif
