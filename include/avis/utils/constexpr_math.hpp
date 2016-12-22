#pragma once

#include <cinttypes>
#include <cmath>


namespace avis {
namespace math {

template <class real_t>
constexpr real_t pi = 3.1415926535897932384626433832795028841971693993751;


namespace cxpr {

template <class int_t>
constexpr auto factorial(int_t x) -> int_t {
    return x <= 1 ? 1 : x * factorial(x - 1);
}

template <class real_t>
constexpr auto pow(real_t x, int exp) -> real_t {
    if (exp == 0)
        return 1;
    else
        return x * pow(x, exp - 1);
}

template <class real_t>
constexpr auto abs(real_t x) -> real_t {
    return x >= 0.0 ? x : -x;
}

template <class real_t>
constexpr auto pmod(real_t n, real_t d) -> real_t {
    real_t m = abs(n) - static_cast<std::uint64_t>(abs(n) / d) * d;
    return n >= 0 ? m : d - m;
}

template <class real_t>
constexpr auto normalize_radian(real_t x) -> real_t {
    x = pmod(x, 2 * pi<real_t>);
    return x <= pi<real_t> ? x : x - 2 * pi<real_t>;
}

template <class real_t>
constexpr auto rad_to_deg(real_t x) -> real_t {
	return (x / pi<real_t>) * 180;
}

template <class real_t>
constexpr auto deg_to_rad(real_t x) -> real_t {
	return (x / 180) * pi<real_t>;
}

template <int N = 5, class real_t>
constexpr auto sin(real_t x) -> real_t {
    x = normalize_radian(x);

    real_t result = x;
    for (std::int64_t n = 1; n < N; n++)
        result += pow(-1.0, n) * (pow(x, 2 * n + 1) / factorial(2 * n + 1));

    return result;
}

template <int N = 5, class real_t>
constexpr auto cos(real_t x) -> real_t {
    x = normalize_radian(x);

    real_t result = 1;
    for (std::int64_t n = 1; n < N; n++)
        result += pow(-1.0, n) * (pow(x, 2 * n) / factorial(2 * n));

    return result;
}

template <class real_t>
constexpr auto sqrt(real_t x) -> real_t {
    return std::sqrt(x);    // TODO: make constexpr
}

} /* namespace cxpr */
} /* namespace math */
} /* namespace avis */
