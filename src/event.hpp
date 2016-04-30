#ifndef ACPROXY_EVENT_HPP_
#define ACPROXY_EVENT_HPP_

namespace ACProxy {
enum Event {
    Read = 0x1,
    Write = 0x2,
    Error = 0x4
};

}

#endif
