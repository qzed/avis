#include <avis/application_base.hpp>
#include <avis/glfw/utils.hpp>
#include <avis/glfw/vulkan_window.hpp>
#include <avis/vulkan/enumerations.hpp>
#include <iostream>


namespace avis {

void application_base::create() try {
    setup_vulkan();
    cb_create();

    window_.set_key_input_callback([this](int key, int scancode, int action, int mods) noexcept {
        this->cb_input_key(key, scancode, action, mods);
    });

    window_.set_resize_callback([this](int width, int height) noexcept {
        this->device_.wait_idle();
        this->swapchain_.recreate();
        this->cb_resize(width, height);
    });

} catch (...) {
    destroy();
    throw;
}

void application_base::destroy() {
    cb_destroy();

    swapchain_.destroy();
    device_.destroy();
    window_.destroy();
    validation_.destroy();
    instance_.destroy();
}

void application_base::run() {
    while (!window_.get_terminate_request()) {
        window_.poll_events();
        cb_display();
    }

    device_.wait_idle();
}


void application_base::setup_vulkan() {
    // create instance
    using std::begin;
    using std::end;

    auto extensions = appinfo_.vulkan_instance_extensions;
    auto layers     = std::vector<const char*>{};

    if (appinfo_.vulkan_validation_enable) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        layers = appinfo_.vulkan_validation_layers;
    }

    auto glfw_extensions = avis::glfw::get_required_instance_extensions();
    extensions.insert(extensions.end(), begin(glfw_extensions), end(glfw_extensions));

    auto app_info = VkApplicationInfo{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.applicationVersion = appinfo_.app_version;
    app_info.pApplicationName   = appinfo_.app_name.c_str();
    app_info.engineVersion      = appinfo_.app_version;
    app_info.pEngineName        = appinfo_.app_name.c_str();
    app_info.apiVersion         = appinfo_.vulkan_version;

    auto instance_info = VkInstanceCreateInfo{};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo        = &app_info;
    instance_info.enabledExtensionCount   = extensions.size();
    instance_info.ppEnabledExtensionNames = extensions.data();
    instance_info.enabledLayerCount       = layers.size();
    instance_info.ppEnabledLayerNames     = layers.data();

    instance_ = vulkan::make_instance(instance_info).move_or_throw();

    // setup debug-report-callback
    if (appinfo_.vulkan_validation_enable) {
        auto debug_info = VkDebugReportCallbackCreateInfoEXT{};
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        debug_info.pfnCallback = &vulkan_debug_callback;
        debug_info.flags       = appinfo_.vulkan_validation_filter;
        debug_info.pUserData   = this;

        validation_ = instance_.create_debug_report_callback(debug_info).move_or_throw();
    }

    // setup window
    window_.create(instance_);

    // setup device
    auto devices = instance_.enumerate_physical_devices().move_or_throw();
    if (devices.empty())
        throw std::runtime_error("No devices with Vulkan support found.");

    device_ = vulkan::make_device(window_.get_surface(), select_physical_device(devices),
            required_vulkan_features(), appinfo_.vulkan_device_extensions, appinfo_.vulkan_validation_layers)
            .move_or_throw();

    // setup swapchain
    swapchain_ = vulkan::make_swapchain(device_, window_).move_or_throw();
}

// debug callbacks

auto application_base::vulkan_debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type,
        uint64_t source_object, size_t location, int32_t code, const char* layer_prefix, const char* message,
        void* user_data) noexcept -> VkBool32 {

    auto self = reinterpret_cast<application_base*>(user_data);
    return self->cb_validation(flags, object_type, source_object, location, code, layer_prefix, message);
}

auto application_base::cb_validation(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, uint64_t, size_t,
        int32_t code, const char* layer_prefix, const char* message) noexcept -> VkBool32 try {
    std::string prefix("");

    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        prefix += "ERROR:";
    }

    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        prefix += "WARNING:";
    }

    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        prefix += "PERFORMANCE:";
    }

    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        prefix += "DEBUG:";
    }

    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        prefix += "INFO:";
    }

    std::cout << prefix << " [" << layer_prefix << "] Code " << code << " : " << message << "\n";

    return false;
} catch (...) {
    std::cerr << "Error occurred in debug-report callback\n";
    return true;
}

// config functions

auto application_base::select_physical_device(std::vector<VkPhysicalDevice> const& devices) const
        -> VkPhysicalDevice
{
    return devices[0];
}

} /* namespace avis */
