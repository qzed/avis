#pragma once

#include <avis/vulkan/vulkan.hpp>
#include <avis/vulkan/handle.hpp>
#include <vector>


namespace avis {
namespace vulkan {

using shader_module = handle<VkShaderModule>;

inline auto make_shader_module(VkDevice device, std::vector<char> code, VkAllocationCallbacks const* alloc = nullptr)
        noexcept -> expected<shader_module>
{
    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = code.size();
    ci.pCode    = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader = {};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateShaderModule(device, &ci, alloc, &shader));

    return make_handle(shader, alloc, [=](auto h, auto a){
        vkDestroyShaderModule(device, h, a);
    });
}

} /* namespace vulkan */
} /* namespace avis */
