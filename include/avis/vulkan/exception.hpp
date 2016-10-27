#pragma once

#include <avis/vulkan/result.hpp>
#include <system_error>


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

} /* namespace vulkan */
} /* namespace avis */


namespace std {

template <>
struct is_error_code_enum<::avis::vulkan::result> : public true_type {};

} /* namespace std */


namespace avis {
namespace vulkan {

class exception : public std::system_error {
public:
    exception(result status) : std::system_error(status) {}

    template <class S>
    exception(S status) : std::system_error(to_result(status)) {}

    exception(result status, std::string const& what) : std::system_error(status, what) {}

    template <class S>
    exception(S status, std::string const& what) : std::system_error(to_result(status), what) {}
};


template <class S>
constexpr void except(S status) {
    if (to_result(status) != result::success) throw exception(to_result(status));
}

#define AVIS_VULKAN_EXCEPT_RETURN(expr) {               \
    ::avis::vulkan::result status = to_result((expr));  \
    if (status != result::success) return status;       \
}


} /* namespace vulkan */
} /* namespace avis */
