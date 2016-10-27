#pragma once

#include <avis/vulkan/vulkan.hpp>
#include <avis/utils/enums.hpp>

#include <system_error>
#include <string>


namespace avis {
namespace vulkan {

enum class result {
    success                        = VK_SUCCESS,
    not_ready                      = VK_NOT_READY,
    timeout                        = VK_TIMEOUT,
    event_set                      = VK_EVENT_SET,
    event_reset                    = VK_EVENT_RESET,
    incomplete                     = VK_INCOMPLETE,

    suboptimal_khr                 = VK_SUBOPTIMAL_KHR,

    error_out_of_host_memory       = VK_ERROR_OUT_OF_HOST_MEMORY,
    error_out_of_device_memory     = VK_ERROR_OUT_OF_DEVICE_MEMORY,
    error_initialization_failed    = VK_ERROR_INITIALIZATION_FAILED,
    error_device_lost              = VK_ERROR_DEVICE_LOST,
    error_memory_map_failed        = VK_ERROR_MEMORY_MAP_FAILED,
    error_layer_not_present        = VK_ERROR_LAYER_NOT_PRESENT,
    error_extension_not_present    = VK_ERROR_EXTENSION_NOT_PRESENT,
    error_feature_not_present      = VK_ERROR_FEATURE_NOT_PRESENT,
    error_incompatible_driver      = VK_ERROR_INCOMPATIBLE_DRIVER,
    error_too_many_objects         = VK_ERROR_TOO_MANY_OBJECTS,
    error_format_not_supported     = VK_ERROR_FORMAT_NOT_SUPPORTED,
    error_fragmented_pool          = VK_ERROR_FRAGMENTED_POOL,

    error_surface_lost_khr         = VK_ERROR_SURFACE_LOST_KHR,
    error_native_window_in_use_khr = VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
    error_out_of_date_khr          = VK_ERROR_OUT_OF_DATE_KHR,
    error_incompatible_display_khr = VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,

    error_validation_failed_ext    = VK_ERROR_VALIDATION_FAILED_EXT,

    error_invalid_shader_nv        = VK_ERROR_INVALID_SHADER_NV,
};

inline std::string to_string(result from) {
    switch (from) {
    case result::success:                        return "VK_SUCCESS";
    case result::not_ready:                      return "VK_NOT_READY";
    case result::timeout:                        return "VK_TIMEOUT";
    case result::event_set:                      return "VK_EVENT_SET";
    case result::event_reset:                    return "VK_EVENT_RESET";
    case result::incomplete:                     return "VK_INCOMPLETE";

    case result::suboptimal_khr:                 return "VK_SUBOPTIMAL_KHR";

    case result::error_out_of_host_memory:       return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case result::error_out_of_device_memory:     return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case result::error_initialization_failed:    return "VK_ERROR_INITIALIZATION_FAILED";
    case result::error_device_lost:              return "VK_ERROR_DEVICE_LOST";
    case result::error_memory_map_failed:        return "VK_ERROR_MEMORY_MAP_FAILED";
    case result::error_layer_not_present:        return "VK_ERROR_LAYER_NOT_PRESENT";
    case result::error_extension_not_present:    return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case result::error_feature_not_present:      return "VK_ERROR_FEATURE_NOT_PRESENT";
    case result::error_incompatible_driver:      return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case result::error_too_many_objects:         return "VK_ERROR_TOO_MANY_OBJECTS";
    case result::error_format_not_supported:     return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case result::error_fragmented_pool:          return "VK_ERROR_FRAGMENTED_POOL";

    case result::error_surface_lost_khr:         return "VK_ERROR_SURFACE_LOST_KHR";
    case result::error_native_window_in_use_khr: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case result::error_out_of_date_khr:          return "VK_ERROR_OUT_OF_DATE_KHR";
    case result::error_incompatible_display_khr: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";

    case result::error_validation_failed_ext:    return "VK_ERROR_VALIDATION_FAILED_EXT";

    case result::error_invalid_shader_nv:        return "VK_ERROR_INVALID_SHADER_NV";

    default:                                     return "Unknown result";
    }
}

inline std::string get_message(result from) {
    switch (from) {
    case result::success:                        return "Command successfull";
    case result::not_ready:                      return "Fence or query has not yet been completed";
    case result::timeout:                        return "Wait operation has timed out";
    case result::event_set:                      return "Event signaled";
    case result::event_reset:                    return "Event unsignaled";
    case result::incomplete:                     return "Return array too small";

    case result::suboptimal_khr:                 return "Swapchain no longer matches the surface properties";

    case result::error_out_of_host_memory:       return "Host memory allocation has failed";
    case result::error_out_of_device_memory:     return "Device memory allocation has failed";
    case result::error_initialization_failed:    return "Initialization of an object could not be completed";
    case result::error_device_lost:              return "The logical or physical device has been lost";
    case result::error_memory_map_failed:        return "Mapping of a memory object has failed";
    case result::error_layer_not_present:        return "A requested layer is not present or could not be loaded";
    case result::error_extension_not_present:    return "A requested extension is not present or could not be loaded";
    case result::error_feature_not_present:      return "A requested feature is not supported";
    case result::error_incompatible_driver:      return "The requested version of Vulkan is not supported";
    case result::error_too_many_objects:         return "Too many objects of a certain type have already been created";
    case result::error_format_not_supported:     return "Requested format is not supported by this device";
    case result::error_fragmented_pool:          return "Requested pool allocation has failed due to fragmentation";

    case result::error_surface_lost_khr:         return "Surface is no longer available";
    case result::error_native_window_in_use_khr: return "The specified native window is already in use";
    case result::error_out_of_date_khr:          return "Swapchain is no longer compatible with the surface";
    case result::error_incompatible_display_khr: return "Images cannot be shared between display and swapchain";

    case result::error_validation_failed_ext:    return "A validation layer has detected an invalid API-usage";

    case result::error_invalid_shader_nv:        return "Not a valid shader";

    default:                                     return "Unknown result";
    }
}


constexpr result to_result(VkResult from) noexcept {
    return static_cast<result>(from);
}

constexpr result to_result(result from) noexcept {
    return from;
}


constexpr bool is_error(result r) {
    return utils::as_underlying_type(r) < 0;
}

constexpr bool is_success(result r) {
    return utils::as_underlying_type(r) >= 0;
}

} /* namespace vulkan */
} /* namespace avis */
