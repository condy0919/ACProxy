#pragma once

#include "../cache.hpp"

namespace ACProxy {

constexpr const std::size_t LOCALCACHESIZE = 256;
constexpr const std::size_t LOCALCACHETHRESHOLD = 64 * 1024; // 64KB
constexpr const std::size_t REDISTHRESHOLD = (1UL << 20);
Cache<std::string, std::string, LOCALCACHESIZE, LOCALCACHETHRESHOLD,
      REDISTHRESHOLD>&
getGlobalCache();
}
