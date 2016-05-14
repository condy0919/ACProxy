#include "global.hpp"

namespace ACProxy {
Cache<std::string, std::string, LOCALCACHESIZE, LOCALCACHETHRESHOLD,
      REDISTHRESHOLD>&
getGlobalCache() {
    static Cache<std::string, std::string, LOCALCACHESIZE, LOCALCACHETHRESHOLD,
                 REDISTHRESHOLD> cache;
    return cache;
}
}
