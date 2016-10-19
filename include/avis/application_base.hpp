#pragma once

#include <avis/glfw/vulkan_window.hpp>
#include <avis/vulkan/instance.hpp>
#include <avis/vulkan/device.hpp>
#include <avis/vulkan/swapchain.hpp>

#include <string>
#include <cinttypes>
#include <vector>


namespace avis {

struct application_info {
    std::string   app_name;
    std::uint32_t app_version;

    std::string  window_title;
    unsigned int window_width;
    unsigned int window_height;
    bool         window_resizable;

    std::uint32_t            vulkan_version;
    std::vector<const char*> vulkan_instance_extensions;
    std::vector<const char*> vulkan_device_extensions;
    bool                     vulkan_validation_enable;
    std::vector<const char*> vulkan_validation_layers;
    std::uint32_t            vulkan_validation_filter;

    VkPhysicalDeviceFeatures vulkan_features;
};


class application_base {
public:
    application_base(application_info appinfo)
            : vulkan_features_{}
            , appinfo_{std::move(appinfo)}
            , instance_{nullptr}
            , validation_{}
            , window_{appinfo_.window_title, appinfo_.window_width, appinfo_.window_height, appinfo_.window_resizable}
            , device_{}
            , swapchain_{} {}

    virtual ~application_base() {}

    void create();
    void destroy();

    void run();

    inline auto window()       noexcept -> glfw::vulkan_window&;
    inline auto window() const noexcept -> glfw::vulkan_window const&;

    inline auto get_application_info() const noexcept -> application_info const&;
    inline auto get_instance()         const noexcept -> vulkan::instance const&;
    inline auto get_device()           const noexcept -> vulkan::device const&;
    inline auto get_swapchain()        const noexcept -> vulkan::swapchain const&;

    inline auto required_vulkan_features() noexcept -> VkPhysicalDeviceFeatures&;
    inline auto required_vulkan_features() const noexcept -> VkPhysicalDeviceFeatures const&;

protected:
    virtual void cb_create()  = 0;
    virtual void cb_destroy() = 0;
    virtual void cb_display() = 0;
    virtual void cb_resize(unsigned int width, unsigned int height) noexcept = 0;

    virtual void cb_input_key(int key, int scancode, int action, int mods) noexcept = 0;

    virtual auto cb_validation(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type,
            uint64_t source_object, size_t location, int32_t code, const char* layer_prefix, const char* message)
            noexcept -> VkBool32;

    virtual auto select_physical_device(std::vector<VkPhysicalDevice> const& devices) const -> VkPhysicalDevice;

private:
    void setup_vulkan();

    static auto vulkan_debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type,
            uint64_t source_object, size_t location, int32_t code, const char* layer_prefix, const char* message,
            void* user_data) noexcept -> VkBool32;

protected:
    VkPhysicalDeviceFeatures      vulkan_features_;

private:
    application_info              appinfo_;
    vulkan::instance              instance_;
    vulkan::debug_report_callback validation_;
    glfw::vulkan_window           window_;
    vulkan::device                device_;
    vulkan::swapchain             swapchain_;
};


auto application_base::get_application_info() const noexcept -> application_info const& {
    return appinfo_;
}

auto application_base::window() noexcept -> glfw::vulkan_window& {
    return window_;
}

auto application_base::window() const noexcept -> glfw::vulkan_window const& {
    return window_;
}

auto application_base::get_instance() const noexcept -> vulkan::instance const& {
    return instance_;
}

auto application_base::get_device() const noexcept -> vulkan::device const& {
    return device_;
}

auto application_base::get_swapchain() const noexcept -> vulkan::swapchain const& {
    return swapchain_;
}

auto application_base::required_vulkan_features() noexcept -> VkPhysicalDeviceFeatures& {
    return vulkan_features_;
}

auto application_base::required_vulkan_features() const noexcept -> VkPhysicalDeviceFeatures const& {
    return vulkan_features_;
}

} /* namespace avis */
