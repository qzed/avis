#include <avis/vulkan/device.hpp>


namespace avis {
namespace vulkan {

auto device::create(VkSurfaceKHR surface, VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures const& features,
        std::vector<char const*> const& device_extensions, std::vector<char const*> const& layers,
        VkAllocationCallbacks const* alloc) noexcept -> expected<device> {

    using std::begin;
    using std::end;

    std::vector<char const*> extensions = device_extensions;
    if (std::find(begin(extensions), end(extensions), VK_KHR_SURFACE_EXTENSION_NAME) == end(extensions))
        extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // get queue family properties
    std::uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families{queue_family_count};
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    // get present and graphics queue
    queue graphics_queue;
    queue present_queue;
    {
        bool found_graphics = false;
        bool found_present  = false;

        for (std::uint32_t i = 0; i < queue_family_count && !(found_graphics && found_present); i++) {
            if (queue_families[i].queueCount == 0) continue;

            VkBool32 present_supported = false;
            VkResult status = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_supported);
            if (status != VK_SUCCESS) return to_result(status);

            if (present_supported) {
                present_queue.index = i;
                found_present = true;
            }

            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphics_queue.index = i;
                found_graphics = true;
            }
        }

        if (!found_graphics || !found_present)
            return result::error_feature_not_present;
    }

    // setup queue create infos
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {};
    float priority = 0.f;
    {
        VkDeviceQueueCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.queueFamilyIndex = graphics_queue.index;
        create_info.queueCount       = 1;
        create_info.pQueuePriorities = &priority;
        queue_create_infos.push_back(create_info);
    }

    if (graphics_queue.index != present_queue.index) {
        VkDeviceQueueCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.queueFamilyIndex = present_queue.index;
        create_info.queueCount       = 1;
        create_info.pQueuePriorities = &priority;
        queue_create_infos.push_back(create_info);
    }

    // setup device create info
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount    = queue_create_infos.size();
    create_info.pQueueCreateInfos       = queue_create_infos.data();
    create_info.pEnabledFeatures        = &features;

    create_info.enabledExtensionCount   = extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();
    create_info.enabledLayerCount       = layers.size();
    create_info.ppEnabledLayerNames     = layers.data();

    // actually create device
    VkDevice device_handle = nullptr;
    VkResult status = vkCreateDevice(physical_device, &create_info, alloc, &device_handle);
    if (status != VK_SUCCESS)
        return to_result(status);

    // get the queue indices
    vkGetDeviceQueue(device_handle, graphics_queue.index, 0, &graphics_queue.handle);
    vkGetDeviceQueue(device_handle, present_queue.index, 0, &present_queue.handle);

    // create the device wrapper
    auto logical_device = make_handle(device_handle, alloc, [](auto handle, auto alloc){
        vkDestroyDevice(handle, alloc);
    });

    return {result::success, {physical_device, logical_device.move(), graphics_queue, present_queue}};
}

} /* namespace vulkan */
} /* namespace avis */
