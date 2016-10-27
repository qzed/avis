#pragma once

#include <avis/vulkan/vulkan.hpp>
#include <avis/vulkan/handle.hpp>


namespace avis {
namespace vulkan {

using semaphore = handle<VkSemaphore>;

inline auto make_semaphore(VkDevice device, VkSemaphoreCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<semaphore>
{
    VkSemaphore handle = nullptr;
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateSemaphore(device, &create_info, nullptr, &handle));

    return vulkan::make_handle(handle, nullptr, [=](auto h, auto a){
        vkDestroySemaphore(device, h, a);
    });
}

inline auto make_semaphore(VkDevice device, VkAllocationCallbacks const* alloc = nullptr) noexcept
        -> expected<semaphore>
{
    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    return make_semaphore(device, create_info, alloc);
}

} /* namespace vulkan */
} /* namespace avis */
