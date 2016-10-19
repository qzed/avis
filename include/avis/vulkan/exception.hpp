#pragma once

#include <avis/vulkan/result.hpp>
#include <system_error>


#define AVIS_VKEXCEPT(result)                       \
{                                                   \
    using ::avis::vulkan::to_result;                \
    ::avis::vulkan::result r = to_result((result))  \
    if (!r) throw ::avis::vulkan::exception(r);     \
}


namespace avis {
namespace vulkan {
namespace detail {

class vulkan_error_category : public std::error_category {
    virtual const char* name() const noexcept override {
        return "vulkan";
    }

    virtual std::string message(int condition) const noexcept override {
        result r = static_cast<result>(condition);
        return get_message(r) + " (" + to_string(r) + ").";
    }
};

} /* namespace detail */


inline const std::error_category& vulkan_category() noexcept {
    static detail::vulkan_error_category category;
    return category;
}

inline std::error_code make_error_code(result from) noexcept {
  return std::error_code(static_cast<int>(from), vulkan_category());
}

inline std::error_condition make_error_condition(result from) noexcept {
  return std::error_condition(static_cast<int>(from), vulkan_category());
}


class exception : public std::system_error {
public:
    using std::system_error::system_error;
};


} /* namespace vulkan */
} /* namespace avis */


namespace std {

template <>
struct is_error_code_enum<::avis::vulkan::result> : public true_type {};

} /* namespace std */
