#pragma once

#include "log.hpp"
#include <hiredis/hiredis.h>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <list>
#include <memory>
#include <unordered_map>

namespace ACProxy {
class RedisConnector {
public:
    RedisConnector(const char* ip, std::uint16_t port)
        : ip_(ip), port_(port) {}

    bool connect() {
        context_.reset(redisConnect(ip_, port_), redisFree);
        return static_cast<bool>(context_);
    }

    bool connect(int sec, int ms) {
        __extension__ struct timeval tv = {
            .tv_sec = sec,
            .tv_usec = ms
        };
        context_.reset(redisConnectWithTimeout(ip_, port_, tv), redisFree);
        return static_cast<bool>(context_);
    }

    bool reconnect() {
        return redisReconnect(context_.get()) == REDIS_OK;
    }

    template <typename... Ts>
    std::shared_ptr<redisReply> command(Ts&&... ts) {
        return std::shared_ptr<redisReply>(
            static_cast<redisReply*>(
                redisCommand(context_.get(), std::forward<Ts>(ts)...)),
            [](redisReply* p) { freeReplyObject(p); });
    }

private:
    std::shared_ptr<redisContext> context_;
    const char* ip_;
    std::uint16_t port_;
};

// more recently used items are put at the begin
template <typename KeyT, typename ValueT, std::size_t Size>
class LruStrategy {
public:
    bool touch(const KeyT& k) {
        if (vs.count(k) > 0) {
            auto pair_ = vs[k];
            ks.splice(ks.begin(), ks, pair_.second);
            pair_.second = ks.begin();
            vs[k] = pair_;
            return true;
        }
        return false;
    }

    boost::optional<ValueT> get(const KeyT& k) {
        if (!touch(k)) {
            return {};
        }
        return vs[k].first;
    }

    void insert(const KeyT& k, const ValueT& v) {
        if (!touch(k)) {
            if (full()) {
                auto rmd = ks.back();
                remove(rmd);
            }

            auto iter = ks.insert(ks.begin(), k);
            vs[k] = std::make_pair(v, iter);
        }
    }

    void remove(const KeyT& k) {
        if (vs.count(k) > 0) {
            auto pair_ = vs[k];
            auto iter = pair_.second;
            ks.erase(iter);
            vs.erase(k);
        }
    }

    void clear() {
        ks.clear();
        vs.clear();
    }

    bool full() const {
        return size() == Size;
    }

    bool empty() const {
        return size() == 0;
    }

    std::size_t size() const {
        return ks.size();
    }

private:
    std::list<KeyT> ks;
    std::unordered_map<KeyT, std::pair<ValueT, typename std::list<KeyT>::iterator>> vs;
};

template <typename KeyT, typename ValueT, std::size_t Size>
class FifoStrategy {
public:
    bool touch(const KeyT& k) {
        return true;
    }

    boost::optional<ValueT> get(const KeyT& k) {
        if (vs.count(k) > 0) {
            return vs[k].first;
        }
        return {};
    }

    void insert(const KeyT& k, const ValueT& v) {
        if (full()) {
            auto rmd = ks.back();
            remove(rmd);
        }

        auto iter = ks.insert(ks.begin(), k);
        vs[k] = std::make_pair(v, iter);
    }

    void remove(const KeyT& k) {
        if (vs.count(k) > 0) {
            auto pair_ = vs[k];
            auto iter = pair_.second;
            ks.erase(iter);
            vs.erase(k);
        }
    }

    void clear() {
        ks.clear();
        vs.clear();
    }

    bool full() const {
        return size() == Size;
    }

    bool empty() const {
        return size() == 0;
    }

    std::size_t size() const {
        return ks.size();
    }

private:
    std::list<KeyT> ks;
    std::unordered_map<KeyT, std::pair<ValueT, typename std::list<KeyT>::iterator>> vs;
};

template <typename KeyT, typename ValueT, std::size_t Size,
          typename Strategy = LruStrategy<KeyT, ValueT, Size>>
class LocalCache {
public:
    LocalCache() : impl(std::make_shared<Strategy>()) {}

    bool touch(const KeyT& k) {
        return impl->touch(k);
    }

    boost::optional<ValueT> get(const KeyT& k) {
        return impl->get(k);
    }

    void insert(const KeyT& k, const ValueT& v) {
        impl->insert(k, v);
    }

    void remove(const KeyT& k) {
        impl->remove(k);
    }

    void clear() {
        impl->clear();
    }

    bool full() const {
        return impl->full();
    }

    bool empty() const {
        return impl->empty();
    }

    std::size_t size() const {
        return impl->size();
    }

private:
    std::shared_ptr<Strategy> impl;
};

template <typename KeyT, typename ValueT, std::size_t LocalCacheSize,
          std::size_t LocalCacheThreshold, std::size_t RedisThreshold>
class Cache {
public:
    Cache() : redis_connector_("127.0.0.1", 6379) {
        redis_connector_.connect();
    }

    boost::optional<ValueT> get(const KeyT& k) {
        boost::optional<ValueT> ret = local_cache_.get(k);
        if (!ret) {
            auto res = redis_connector_.command("get %s", k); // FIXME
            if (!res->str) {
                return {};
            }
            return boost::lexical_cast<ValueT>(res->str);
        }
        return ret;
    }

    void set(const KeyT& k, const ValueT& v) {
        if (v.size() <= LocalCacheThreshold) {
            local_cache_.insert(k, v);
        } else if (v.size() <= RedisThreshold) {
            redis_connector_.command("set %s %s", k, v); // FIXME
        } else {
            LOG_ACPROXY_WARNING("sizeof value is larger than threshold");
        }
    }

private:
    LocalCache<KeyT, ValueT, LocalCacheSize> local_cache_;
    RedisConnector redis_connector_;
};

std::string keygen(std::string host, int port, std::string uri,
                   std::string method = "GET");
}
