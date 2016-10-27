#pragma once

#include <avis/vulkan/vulkan.hpp>
#include <avis/vulkan/handle.hpp>


namespace avis {
namespace vulkan {

using render_pass = handle<VkRenderPass>;

inline auto make_render_pass(VkDevice device, VkRenderPassCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<render_pass>
{
    VkRenderPass renderpass = nullptr;
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateRenderPass(device, &create_info, nullptr, &renderpass));

    return vulkan::make_handle(renderpass, nullptr, [=](auto h, auto a){
        vkDestroyRenderPass(device, h, a);
    });
}

} /* namespace vulkan */
} /* namespace avis */
