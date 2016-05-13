#pragma once

#include <list>
#include <memory>
#include <unordered_map>

namespace ACProxy {
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

    ValueT get(const KeyT& k) {
        if (!touch(k)) {
            return ValueT();
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

    ValueT get(const KeyT& k) {
        if (vs.count(k) > 0) {
            return vs[k].first;
        }
        return ValueT();
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
class Cache {
public:
    Cache() : impl(std::make_shared<Strategy>()) {}

    bool touch(const KeyT& k) {
        return impl->touch(k);
    }

    ValueT get(const KeyT& k) {
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
}
