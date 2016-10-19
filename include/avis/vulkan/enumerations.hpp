#pragma once

#include <avis/vulkan/expected.hpp>
#include <vector>


namespace avis {
namespace vulkan {

template <class Allocator = std::vector<VkSurfaceFormatKHR>::allocator_type>
inline auto get_physical_device_surface_formats_khr(VkPhysicalDevice device, VkSurfaceKHR surface,
        Allocator const& alloc = {}) -> expected<std::vector<VkSurfaceFormatKHR, Allocator>> {

    VkResult status;
    std::uint32_t count = 0;

    status = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
    if (status != VK_SUCCESS) return to_result(status);

    std::vector<VkSurfaceFormatKHR, Allocator> formats{count, alloc};

    status = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());
    if (status != VK_SUCCESS) return to_result(status);

    return formats;
}

template <class Allocator = std::vector<VkPresentModeKHR>::allocator_type>
inline auto get_physical_device_surface_present_modes_khr(VkPhysicalDevice device, VkSurfaceKHR surface,
        Allocator const& alloc = {}) -> expected<std::vector<VkPresentModeKHR, Allocator>> {

    VkResult status;
    std::uint32_t count = 0;

    status = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
    if (status != VK_SUCCESS) return to_result(status);

    std::vector<VkPresentModeKHR, Allocator> modes{count, alloc};

    status = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data());
    if (status != VK_SUCCESS) return to_result(status);

    return modes;
}

template <class Allocator = std::vector<VkImage>::allocator_type>
inline auto get_swapchain_images_khr(VkDevice device, VkSwapchainKHR swapchain,
        Allocator const& alloc = {}) -> expected<std::vector<VkImage, Allocator>> {

    VkResult status;
    std::uint32_t count = 0;

    status = vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
    if (status != VK_SUCCESS) return to_result(status);

    std::vector<VkImage, Allocator> images{count, alloc};

    status = vkGetSwapchainImagesKHR(device, swapchain, &count, images.data());
    if (status != VK_SUCCESS) return to_result(status);

    return images;
}

} /* namespace vulkan */
} /* namespace avis */
