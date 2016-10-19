#pragma once

#include <avis/vulkan/result.hpp>
#include <avis/vulkan/exception.hpp>


namespace avis {
namespace vulkan {

template <class T>
class expected {
public:
    using value_type  = T;
    using status_type = vulkan::result;

    expected()
        : status_{result::success}
        , value_{} {}

    expected(expected const&) = default;
    expected(expected&&)      = default;

    expected(value_type& val)
        : status_{result::success}
        , value_{val} {}

    expected(value_type&& val)
        : status_{result::success}
        , value_{std::forward<value_type>(val)} {}

    expected(status_type err)
        : status_(err)
        , value_{} {}

    expected(status_type err, value_type& val)
        : status_{err}
        , value_{val} {}

    expected(status_type err, value_type&& val)
        : status_{err}
        , value_{std::forward<value_type>(val)} {}


    inline auto operator= (const expected& rhs) -> expected& = default;
    inline auto operator= (expected&& rhs)      -> expected& = default;


    inline explicit operator bool() const noexcept;

    inline auto has_succeeded() const noexcept -> bool;
    inline auto has_failed()    const noexcept -> bool;

    inline auto status()       noexcept -> status_type&;
    inline auto status() const noexcept -> status_type const&;

    inline void throw_if_failed() const;

    inline auto value()       noexcept -> value_type&;
    inline auto value() const noexcept -> value_type const&;

    inline auto value_or(value_type& alternative)             noexcept -> value_type&;
    inline auto value_or(value_type const& alternative) const noexcept -> value_type const&;

    inline auto value_or_throw()       -> value_type&;
    inline auto value_or_throw() const -> value_type const&;

    inline auto move() -> value_type&&;
    inline auto move_or(value_type&& alternative) -> value_type&&;
    inline auto move_or_throw() -> value_type&&;

private:
    status_type status_;
    value_type  value_;
};


template <class T>
expected<T>::operator bool() const noexcept {
    return status_ == result::success;
}

template <class T>
auto expected<T>::has_succeeded() const noexcept -> bool {
    return is_success(status_);
}

template <class T>
auto expected<T>::has_failed() const noexcept -> bool {
    return is_error(status_);
}


template <class T>
auto expected<T>::status() noexcept -> status_type& {
    return status_;
}

template <class T>
auto expected<T>::status() const noexcept -> status_type const& {
    return status_;
}


template <class T>
void expected<T>::throw_if_failed() const {
    if (status_ != result::success) throw vulkan::exception{status_};
}


template <class T>
auto expected<T>::value() noexcept -> value_type& {
    return value_;
}

template <class T>
auto expected<T>::value() const noexcept -> value_type const& {
    return value_;
}


template <class T>
auto expected<T>::value_or(value_type& alternative) noexcept -> value_type& {
    return status_ == result::success ? value_ : alternative;
}

template <class T>
auto expected<T>::value_or(const value_type& alternative) const noexcept -> value_type const& {
    return status_ == result::success ? value_ : alternative;
}


template <class T>
auto expected<T>::value_or_throw() -> value_type& {
    if (status_ == result::success)
        return value_;
    else
        throw vulkan::exception{status_};
}

template <class T>
auto expected<T>::value_or_throw() const -> value_type const& {
    if (status_ == result::success)
        return value_;
    else
        throw vulkan::exception{status_};
}


template <class T>
auto expected<T>::move() -> value_type&& {
    return std::move(value_);
}

template <class T>
auto expected<T>::move_or(value_type&& alternative) -> value_type&& {
    return status_ == result::success ? std::move(value_) : std::forward<value_type>(alternative);
}

template <class T>
auto expected<T>::move_or_throw() -> value_type&& {
    if (status_ == result::success)
        return std::move(value_);
    else
        throw vulkan::exception(status_);
}


template <class T>
bool operator== (expected<T> a, expected<T> b) {
    bool s = a.status() == b.status();
    return (s && a) ? a.result() == b.result() : s;
}

template <class T>
bool operator!= (expected<T> a, expected<T> b) {
    return !(a == b);
}

} /* namespace vulkan */
} /* namespace avis */
