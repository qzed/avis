#pragma once

#include <avis/utils/expected.hpp>
#include <avis/vulkan/result.hpp>


namespace avis {
namespace utils {
namespace expected_impl {

template <>
struct status_values<vulkan::result> {
    static constexpr auto success = vulkan::result::success;
    static constexpr bool is_success(vulkan::result status) { return status == vulkan::result::success; }
    static constexpr bool is_error(vulkan::result status)   { return status != vulkan::result::success; }
    static inline    auto exception(vulkan::result status)  { return vulkan::exception{status}; }
};

} /* namespace expected_impl */
} /* namespace utils */


namespace vulkan {

template <class T>
using expected = utils::expected<T, result>;

} /* namespace vulkan */
} /* namespace avis */
