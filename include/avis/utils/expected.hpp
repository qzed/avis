#pragma once

#include <avis/vulkan/result.hpp>
#include <avis/vulkan/exception.hpp>


namespace avis {
namespace utils {

namespace expected_impl {

template<class S>
struct status_values;

}

template <class T, class S>
class expected {
public:
    using status_values = expected_impl::status_values<S>;

    using value_type  = T;
    using status_type = S;

    expected()
        : status_{status_values::success}
        , value_{} {}

    expected(expected const&) = default;
    expected(expected&&)      = default;

    expected(value_type& val)
        : status_{status_values::success}
        , value_{val} {}

    expected(value_type&& val)
        : status_{status_values::success}
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


template <class T, class S>
expected<T, S>::operator bool() const noexcept {
    return status_values::is_success(status_);
}

template <class T, class S>
auto expected<T, S>::has_succeeded() const noexcept -> bool {
    return status_values::is_success(status_);
}

template <class T, class S>
auto expected<T, S>::has_failed() const noexcept -> bool {
    return status_ != status_values::is_error(status_);
}


template <class T, class S>
auto expected<T, S>::status() noexcept -> status_type& {
    return status_;
}

template <class T, class S>
auto expected<T, S>::status() const noexcept -> status_type const& {
    return status_;
}


template <class T, class S>
void expected<T, S>::throw_if_failed() const {
    if (has_failed()) throw status_values::exception(status_);
}


template <class T, class S>
auto expected<T, S>::value() noexcept -> value_type& {
    return value_;
}

template <class T, class S>
auto expected<T, S>::value() const noexcept -> value_type const& {
    return value_;
}


template <class T, class S>
auto expected<T, S>::value_or(value_type& alternative) noexcept -> value_type& {
    return status_values::is_success(status_) ? value_ : alternative;
}

template <class T, class S>
auto expected<T, S>::value_or(value_type const& alternative) const noexcept -> value_type const& {
    return status_values::is_success(status_) ? value_ : alternative;
}


template <class T, class S>
auto expected<T, S>::value_or_throw() -> value_type& {
    if (status_values::is_success(status_))
        return value_;
    else
        throw status_values::exception(status_);
}

template <class T, class S>
auto expected<T, S>::value_or_throw() const -> value_type const& {
    if (status_values::is_success(status_))
        return value_;
    else
        throw status_values::exception(status_);
}


template <class T, class S>
auto expected<T, S>::move() -> value_type&& {
    return std::move(value_);
}

template <class T, class S>
auto expected<T, S>::move_or(value_type&& alternative) -> value_type&& {
    return status_values::is_success(status_) ? std::move(value_) : std::forward<value_type>(alternative);
}

template <class T, class S>
auto expected<T, S>::move_or_throw() -> value_type&& {
    if (status_values::is_success(status_))
        return std::move(value_);
    else
        throw status_values::exception(status_);
}


template <class T, class S>
bool operator== (expected<T, S> a, expected<T, S> b) {
    bool s = a.status() == b.status();
    return (s && a) ? a.result() == b.result() : s;
}

template <class T, class S>
bool operator!= (expected<T, S> a, expected<T, S> b) {
    return !(a == b);
}

} /* namespace utils */
} /* namespace avis */
