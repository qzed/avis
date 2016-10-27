#pragma once

#include <avis/vulkan/expected.hpp>
#include <vector>


namespace avis {
namespace vulkan {

template <class Allocator = std::vector<VkSurfaceFormatKHR>::allocator_type>
inline auto get_physical_device_surface_formats_khr(VkPhysicalDevice device, VkSurfaceKHR surface,
        Allocator const& alloc = {}) -> expected<std::vector<VkSurfaceFormatKHR, Allocator>> {

    std::uint32_t count = 0;
    AVIS_VULKAN_EXCEPT_RETURN(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr));

    std::vector<VkSurfaceFormatKHR, Allocator> formats{count, alloc};
    AVIS_VULKAN_EXCEPT_RETURN(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data()));

    return formats;
}

template <class Allocator = std::vector<VkPresentModeKHR>::allocator_type>
inline auto get_physical_device_surface_present_modes_khr(VkPhysicalDevice device, VkSurfaceKHR surface,
        Allocator const& alloc = {}) -> expected<std::vector<VkPresentModeKHR, Allocator>> {

    std::uint32_t count = 0;
    AVIS_VULKAN_EXCEPT_RETURN(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr));

    std::vector<VkPresentModeKHR, Allocator> modes{count, alloc};
    AVIS_VULKAN_EXCEPT_RETURN(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data()));

    return modes;
}

template <class Allocator = std::vector<VkImage>::allocator_type>
inline auto get_swapchain_images_khr(VkDevice device, VkSwapchainKHR swapchain,
        Allocator const& alloc = {}) -> expected<std::vector<VkImage, Allocator>> {

    std::uint32_t count = 0;
    AVIS_VULKAN_EXCEPT_RETURN(vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr));

    std::vector<VkImage, Allocator> images{count, alloc};
    AVIS_VULKAN_EXCEPT_RETURN(vkGetSwapchainImagesKHR(device, swapchain, &count, images.data()));

    return images;
}

} /* namespace vulkan */
} /* namespace avis */
