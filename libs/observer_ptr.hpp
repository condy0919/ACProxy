#pragma once

#include <algorithm>
#include <functional>
#include <type_traits>

namespace std {

template <typename T>
class observer_ptr {
public:
    using element_type = T;
    using pointer = std::add_pointer_t<element_type>;
    using reference = std::add_lvalue_reference_t<element_type>;

    constexpr observer_ptr() noexcept : ptr_(nullptr) {}

    constexpr observer_ptr(nullptr_t) noexcept : ptr_(nullptr) {}

    constexpr observer_ptr(pointer p) noexcept : ptr_(p) {}

    template <typename U>
    constexpr observer_ptr(observer_ptr<U> rhs) noexcept : ptr_(rhs.ptr_) {}

    constexpr pointer get() const noexcept {
        return ptr_;
    }

    constexpr reference operator*() const {
        return *ptr_;
    }

    constexpr pointer operator->() const noexcept {
        return ptr_;
    }

    constexpr explicit operator bool() const noexcept {
        return ptr_;
    }

    constexpr explicit operator pointer() const noexcept {
        return ptr_;
    }

    constexpr pointer release() noexcept {
        pointer p(ptr_);
        reset();
        return p;
    }

    constexpr void reset(pointer p = nullptr) noexcept {
        ptr_ = p;
    }

    constexpr void swap(observer_ptr& rhs) noexcept {
        using std::swap;
        swap(ptr_, rhs.ptr_);
    }

private:
    pointer ptr_;
};

template <typename T>
inline void swap(observer_ptr<T>& lhs, observer_ptr<T>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename T>
inline observer_ptr<T> make_observer(T* p) noexcept {
    return observer_ptr<T>(p);
}

template <typename T, typename U>
inline bool operator==(const observer_ptr<T>& lhs, const observer_ptr<U>& rhs) {
    return lhs.get() == rhs.get();
}

template <typename T, typename U>
inline bool operator!=(const observer_ptr<T>& lhs, const observer_ptr<U>& rhs) {
    return !(lhs == rhs);
}

template <typename T>
inline bool operator==(const observer_ptr<T>& lhs, nullptr_t) noexcept {
    return !lhs;
}

template <typename T>
inline bool operator==(nullptr_t, const observer_ptr<T>& rhs) noexcept {
    return !rhs;
}

template <typename T>
inline bool operator!=(const observer_ptr<T>& lhs, nullptr_t) noexcept {
    return (bool)lhs;
}

template <typename T>
inline bool operator!=(nullptr_t, const observer_ptr<T>& rhs) noexcept {
    return (bool)rhs;
}

template <typename T, typename U>
inline bool operator<(const observer_ptr<T>& lhs, const observer_ptr<U>& rhs) {
    using W = std::common_type_t<std::decay_t<T>, std::decay_t<U>>;
    return std::less<W>(lhs.get(), rhs.get());
}

template <typename T, typename U>
inline bool operator>(const observer_ptr<T>& lhs, const observer_ptr<U>& rhs) {
    return rhs < lhs;
}

template <typename T, typename U>
inline bool operator<=(const observer_ptr<T>& lhs, const observer_ptr<U>& rhs) {
    return !(rhs < lhs);
}

template <typename T, typename U>
inline bool operator>=(const observer_ptr<T>& lhs, const observer_ptr<U>& rhs) {
    return !(lhs < rhs);
}

template <typename T>
struct hash<observer_ptr<T>> {
    size_t operator()(observer_ptr<T> p) {
        return hash<std::add_pointer_t<T>>(p.get());
    }
};
}
