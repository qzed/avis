#include <avis/vulkan/instance.hpp>


namespace avis {
namespace vulkan {

auto instance::create_debug_report_callback(VkDebugReportCallbackCreateInfoEXT& create_info,
        const VkAllocationCallbacks *alloc) noexcept -> expected<debug_report_callback> {

    // load extension specific functions
    if (!fn_create_debug_report_callback_ext_)
        fn_create_debug_report_callback_ext_ = AVIS_VULKAN_GET_INSTANCE_PROC_ADDR(*this, vkCreateDebugReportCallbackEXT);

    if (!fn_destroy_debug_report_callback_ext_)
        fn_destroy_debug_report_callback_ext_ = AVIS_VULKAN_GET_INSTANCE_PROC_ADDR(*this, vkDestroyDebugReportCallbackEXT);

    if (!fn_create_debug_report_callback_ext_ || !fn_destroy_debug_report_callback_ext_)
        return result::error_extension_not_present;

    // create callback
    VkDebugReportCallbackEXT callback = nullptr;
    result status = to_result(fn_create_debug_report_callback_ext_(handle_, &create_info, alloc, &callback));
    if (status == result::success) {
        auto handle = make_handle(callback, alloc, [=](auto... p) {
            fn_destroy_debug_report_callback_ext_(handle_, p...);
        });
        if (handle)
            return handle.move();
        else
            return handle.status();
    } else {
        return {status, debug_report_callback{}};
    }
}

} /* namespace vulkan */
} /* namespace avis */
