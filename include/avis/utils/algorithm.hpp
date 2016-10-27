#pragma once

#include <utility>


namespace avis {
namespace utils {

template <class T>
constexpr T const& clamp(T const& val, T const& min, T const& max) {
    return (val < min) ? min : ((val > max) ? max : val);
}

} /* namespace utils */
} /* namespace avis */
