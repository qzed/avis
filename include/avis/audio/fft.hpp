#pragma once

#include <avis/utils/constexpr_math.hpp>

#include <array>
#include <complex>
#include <utility>


namespace avis {
namespace audio {


template <class Fn, std::size_t... Indices>
constexpr auto make_array(Fn fn, std::integer_sequence<std::size_t, Indices...> seq)
        -> std::array<typename std::result_of<Fn(std::size_t)>::type, sizeof...(Indices)>
{
    return {{ fn(Indices)... }};
}

template <std::size_t N, class Fn>
constexpr auto make_array(Fn fn) -> std::array<typename std::result_of<Fn(std::size_t)>::type, N> {
    return make_array(fn, std::make_integer_sequence<std::size_t, N>());
}


constexpr auto bitcount(std::size_t n) -> int {
    int c = 0;
    for (; n; c++)
        n &= n - 1;

    return c;
}

constexpr auto bitrev(std::size_t n, std::size_t bits) -> std::size_t {
    std::size_t j1 = n;
    std::size_t k = 0;

    for (std::size_t i = 0; i <= bits; i++) {
        auto j2 = j1 >> 1;
        k  = ( k << 1 ) + j1 - ( j2 << 1 );
        j1 = j2;
    }

    return k;
}


template <std::size_t N>
constexpr auto io_shuffle_table_entry(std::size_t num) -> std::size_t {
    return bitrev(num, bitcount(N-1) - 1);
}

template <std::size_t N>
constexpr auto io_shuffle_table() -> std::array<std::size_t, N> {
    return make_array<N>(io_shuffle_table_entry<N>);
}


template <std::size_t N, class real_t>
constexpr auto hanning_window_table_entry(std::size_t i) -> real_t {
    return 0.5 * (1.0 - math::cxpr::cos((2.0 * math::pi<real_t> * i) / (N - 1.0)));
}

template <std::size_t N, class real_t>
constexpr auto hanning_window_table() -> std::array<real_t, N> {
    return make_array<N>(hanning_window_table_entry<N, real_t>);
}


template <std::size_t N, class real_t>
constexpr auto fft_root_table_entry(std::size_t k) -> std::complex<real_t> {
    real_t arg = 2 * math::pi<real_t> * k / N;
    return {math::cxpr::cos(arg), -math::cxpr::sin(arg)};
}

template <std::size_t N, class real_t>
constexpr auto fft_root_table() -> std::array<std::complex<real_t>, N/2> {
    return make_array<N/2>(fft_root_table_entry<N, real_t>);
}


template<std::size_t N, class real_t>
void rfft(real_t* data, real_t* dst) {
    constexpr static auto lut_shuffle = io_shuffle_table<N>();
    constexpr static auto lut_window  = hanning_window_table<N, real_t>();
    constexpr static auto lut_roots   = fft_root_table<N, real_t>();

    constexpr static auto scale = 1.0 / math::cxpr::sqrt(static_cast<real_t>(N));

    // convert real-data to complex data and shuffle
    auto buffer = std::array<std::complex<real_t>, N>();
    for (std::size_t i = 0; i < N; i++)
        buffer[lut_shuffle[i]] = {data[i] * lut_window[i]};

    // perform fft
    for (std::uint64_t groups = N/2; groups > 0; groups >>=1) {
        auto values = N / groups;
        auto pairs = values / 2;

        for (std::uint64_t group = 0; group < groups; group++) {
            auto index_a = group * values;
            auto index_b = index_a + pairs;

            for (std::uint64_t pair = 0; pair < pairs; pair++, index_a++, index_b++) {
                auto tmp = buffer[index_b] * lut_roots[pair * groups];
                buffer[index_b] = buffer[index_a] - tmp;
                buffer[index_a] = buffer[index_a] + tmp;
            }
        }
    }

    // calculate magnitude
    for (std::size_t i = 0; i < N; i++)
        dst[i] = std::abs(buffer[i]) * scale;
}


} /* namespace audio */
} /* namespace avis */
