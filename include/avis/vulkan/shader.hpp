#pragma once

#include <avis/vulkan/vulkan.hpp>
#include <avis/vulkan/handle.hpp>
#include <avis/vulkan/exception.hpp>
#include <vector>


namespace avis {
namespace vulkan {

inline auto create_shader_module(VkDevice device, std::vector<char> code, VkAllocationCallbacks const* alloc = nullptr)
        -> handle<VkShaderModule>
{
    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = code.size();
    ci.pCode    = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader = {};
    VkResult status = vkCreateShaderModule(device, &ci, alloc, &shader);
    if (status != VK_SUCCESS)
        throw vulkan::exception(to_result(status));

    return make_handle(shader, alloc, [=](auto h, auto a){
        vkDestroyShaderModule(device, h, a);
    }).move_or_throw();
}

} /* namespace vulkan */
} /* namespace avis */
