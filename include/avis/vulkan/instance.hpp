#pragma once

#include <avis/vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <avis/vulkan/expected.hpp>
#include <avis/vulkan/handle.hpp>
#include <algorithm>
#include <vector>


#define AVIS_VULKAN_GET_INSTANCE_PROC_ADDR(instance, name) \
    reinterpret_cast<PFN_##name>((instance).get_proc_address(#name));


namespace avis {
namespace vulkan {

using debug_report_callback = handle<VkDebugReportCallbackEXT>;


class instance {
public:
    using vulkan_instance_proc = void(*)(void);

    inline static auto create(VkInstanceCreateInfo const& create_info, const VkAllocationCallbacks* alloc = nullptr)
            noexcept -> expected<instance>;

    instance() : instance{nullptr, nullptr} {}

    instance(VkInstance handle, VkAllocationCallbacks const* alloc = nullptr)
        : handle_{handle}
        , alloc_{alloc}
        , fn_create_debug_report_callback_ext_{nullptr}
        , fn_destroy_debug_report_callback_ext_{nullptr} {}

    instance(instance&& other)
        : handle_{std::exchange(other.handle_, nullptr)}
        , alloc_{std::exchange(other.alloc_, nullptr)}
        , fn_create_debug_report_callback_ext_{std::exchange(other.fn_create_debug_report_callback_ext_, nullptr)}
        , fn_destroy_debug_report_callback_ext_{std::exchange(other.fn_destroy_debug_report_callback_ext_, nullptr)}
        {}

    instance(instance const& other) = delete;

    ~instance() { destroy(); }

    inline void destroy() noexcept;
    inline void destroy(VkAllocationCallbacks const* alloc) noexcept;

    inline auto operator= (instance const& rhs) noexcept -> instance& = delete;
    inline auto operator= (instance&& rhs) noexcept -> instance&;

    inline auto get_handle() const noexcept -> VkInstance;

    inline auto allocator() noexcept -> VkAllocationCallbacks const* &;
    inline auto allocator() const noexcept -> VkAllocationCallbacks const* const&;

    inline auto release() noexcept -> VkInstance;

    // instance functions
    inline auto get_proc_address(char const* name) noexcept -> vulkan_instance_proc;

    auto create_debug_report_callback(VkDebugReportCallbackCreateInfoEXT& create_info,
            VkAllocationCallbacks const* alloc) noexcept -> expected<debug_report_callback>;

    template <class Allocator = std::vector<VkPhysicalDevice>::allocator_type>
    inline auto enumerate_physical_devices(Allocator const& alloc = {}) -> expected<std::vector<VkPhysicalDevice, Allocator>>;

private:
    VkInstance handle_;
    VkAllocationCallbacks const* alloc_;

    // extension specific functions
    PFN_vkCreateDebugReportCallbackEXT  fn_create_debug_report_callback_ext_;
    PFN_vkDestroyDebugReportCallbackEXT fn_destroy_debug_report_callback_ext_;
};


auto instance::create(VkInstanceCreateInfo const& create_info, const VkAllocationCallbacks* alloc) noexcept
        -> expected<instance> {
    VkInstance inst = nullptr;
    VkResult result = vkCreateInstance(&create_info, alloc, &inst);
    return {to_result(result), instance{inst, alloc}};
}

void instance::destroy() noexcept {
    destroy(alloc_);
}

void instance::destroy(VkAllocationCallbacks const* alloc) noexcept {
    if (!handle_) return;

    vkDestroyInstance(handle_, alloc);
    handle_ = nullptr;
    alloc_ = nullptr;
}

auto instance::operator= (instance&& rhs) noexcept -> instance& {
    destroy();
    handle_ = std::exchange(rhs.handle_, nullptr);
    alloc_  = std::exchange(rhs.alloc_, nullptr);
    return *this;
}

auto instance::get_handle() const noexcept -> VkInstance {
    return handle_;
}

auto instance::allocator() noexcept -> VkAllocationCallbacks const* & {
    return alloc_;
}

auto instance::allocator() const noexcept -> VkAllocationCallbacks const* const& {
    return alloc_;
}

auto instance::release() noexcept -> VkInstance  {
    return std::exchange(handle_, nullptr);
}


auto instance::get_proc_address(const char *name) noexcept -> vulkan_instance_proc {
    return glfwGetInstanceProcAddress(handle_, name);
}


template <class Allocator>
auto instance::enumerate_physical_devices(Allocator const& alloc) -> expected<std::vector<VkPhysicalDevice, Allocator>> {
    std::uint32_t count = 0;

    result status = to_result(vkEnumeratePhysicalDevices(handle_, &count, nullptr));
    if (status != result::success)
        return status;

    std::vector<VkPhysicalDevice> devices{count};
    status = to_result(vkEnumeratePhysicalDevices(handle_, &count, devices.data()));
    if (status != result::success)
        return status;

    return devices;
}

} /* namespace vulkan */
} /* namespace avis */
