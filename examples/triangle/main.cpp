#include "triangle_example.hpp"

#include <avis/glfw/initializer.hpp>
#include <iostream>


int main() {
    avis::glfw::scope_initializer glfw_initializer;

    avis::application_info appinfo = {};
    appinfo.app_name                   = "avis-vk";
    appinfo.app_version                = VK_MAKE_VERSION(1, 0, 0);

    appinfo.window_title               = "Vulkan Triangle Example";
    appinfo.window_width               = 1920;
    appinfo.window_height              = 1080;
    appinfo.window_resizable           = true;

    appinfo.vulkan_version             = VK_MAKE_VERSION(1, 0, 0);
    appinfo.vulkan_instance_extensions = {};
    appinfo.vulkan_device_extensions   = {};
    appinfo.vulkan_validation_enable   = true;
    appinfo.vulkan_validation_layers   = { "VK_LAYER_LUNARG_standard_validation" };
    appinfo.vulkan_validation_filter   = VK_DEBUG_REPORT_ERROR_BIT_EXT
                                       | VK_DEBUG_REPORT_WARNING_BIT_EXT
                                       | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
                                       | VK_DEBUG_REPORT_DEBUG_BIT_EXT
                                       | VK_DEBUG_REPORT_INFORMATION_BIT_EXT;

    avis::triangle_example app{appinfo};
    app.create();
    app.run();
}
