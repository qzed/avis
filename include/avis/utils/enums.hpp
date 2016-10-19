#pragma once

#include <type_traits>


namespace avis {
namespace enums {

template <typename T>
constexpr auto as_underlying_type(T t) -> std::underlying_type_t<T> {
    return static_cast<std::underlying_type_t<T>>(t);
}

} /* namespace enums */
} /* namespace avis */
