#pragma once

#include <avis/vulkan/vulkan.hpp>
#include <avis/vulkan/handle.hpp>

#include <vector>


namespace avis {
namespace vulkan {

using render_pass           = handle<VkRenderPass>;
using semaphore             = handle<VkSemaphore>;
using shader_module         = handle<VkShaderModule>;
using descriptor_set_layout = handle<VkDescriptorSetLayout>;
using descriptor_pool       = handle<VkDescriptorPool>;
using pipeline_layout       = handle<VkPipelineLayout>;
using pipeline              = handle<VkPipeline>;
using framebuffer           = handle<VkFramebuffer>;
using command_pool          = handle<VkCommandPool>;
using image_view            = handle<VkImageView>;
using sampler               = handle<VkSampler>;


inline auto make_render_pass(VkDevice device, VkRenderPassCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<render_pass>
{
    auto handle = VkRenderPass{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateRenderPass(device, &create_info, alloc, &handle));

    return vulkan::make_handle(handle, alloc, [=](auto... p){
        vkDestroyRenderPass(device, p...);
    });
}

inline auto make_semaphore(VkDevice device, VkSemaphoreCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<semaphore>
{
    auto handle = VkSemaphore{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateSemaphore(device, &create_info, alloc, &handle));

    return vulkan::make_handle(handle, alloc, [=](auto... p){
        vkDestroySemaphore(device, p...);
    });
}

inline auto make_semaphore(VkDevice device, VkAllocationCallbacks const* alloc = nullptr) noexcept
        -> expected<semaphore>
{
    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    return make_semaphore(device, create_info, alloc);
}

inline auto make_shader_module(VkDevice device, std::vector<char> code, VkAllocationCallbacks const* alloc = nullptr)
        noexcept -> expected<shader_module>
{
    auto create_info = VkShaderModuleCreateInfo{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode    = reinterpret_cast<const uint32_t*>(code.data());

    auto handle = VkShaderModule{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateShaderModule(device, &create_info, alloc, &handle));

    return make_handle(handle, alloc, [=](auto... p){
        vkDestroyShaderModule(device, p...);
    });
}

inline auto make_descriptor_set_layout(VkDevice device, VkDescriptorSetLayoutCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<descriptor_set_layout>
{
    auto handle = VkDescriptorSetLayout{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateDescriptorSetLayout(device, &create_info, alloc, &handle));

    return vulkan::make_handle(handle, alloc, [=](auto... p){
        vkDestroyDescriptorSetLayout(device, p...);
    });
}

inline auto make_descriptor_pool(VkDevice device, VkDescriptorPoolCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<descriptor_pool>
{
    auto handle = VkDescriptorPool{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateDescriptorPool(device, &create_info, alloc, &handle));

    return vulkan::make_handle(handle, alloc, [=](auto... p){
        vkDestroyDescriptorPool(device, p...);
    });
}

inline auto make_pipeline_layout(VkDevice device, VkPipelineLayoutCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<pipeline_layout>
{
    auto handle = VkPipelineLayout{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreatePipelineLayout(device, &create_info, alloc, &handle));

    return vulkan::make_handle(handle, alloc, [=](auto h, auto a){
        vkDestroyPipelineLayout(device, h, a);
    });
}

inline auto make_pipeline(VkDevice device, VkPipelineCache cache, VkGraphicsPipelineCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<pipeline>
{
    auto handle = VkPipeline{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateGraphicsPipelines(device, cache, 1, &create_info, alloc, &handle));

    return vulkan::make_handle(handle, alloc, [=](auto... p){
        vkDestroyPipeline(device, p...);
    }).move_or_throw();
}

inline auto make_framebuffer(VkDevice device, VkFramebufferCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<framebuffer>
{
    auto handle = VkFramebuffer{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateFramebuffer(device, &create_info, alloc, &handle));

    return vulkan::make_handle(handle, alloc, [=](auto... p){
        vkDestroyFramebuffer(device, p...);
    });
}

inline auto make_command_pool(VkDevice device, VkCommandPoolCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<command_pool>
{
    auto handle = VkCommandPool{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateCommandPool(device, &create_info, alloc, &handle));

    return vulkan::make_handle(handle, alloc, [=](auto... p){
        vkDestroyCommandPool(device, p...);
    });
}

inline auto make_image_view(VkDevice device, VkImageViewCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<image_view>
{
    auto handle = VkImageView{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateImageView(device, &create_info, alloc, &handle));

    return vulkan::make_handle(handle, alloc, [=](auto... p){
        vkDestroyImageView(device, p...);
    });
}

inline auto make_sampler(VkDevice device, VkSamplerCreateInfo const& create_info,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<sampler>
{
    auto handle = VkSampler{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateSampler(device, &create_info, alloc, &handle));

    return vulkan::make_handle(handle, alloc, [=](auto... p){
        vkDestroySampler(device, p...);
    });
}

} /* namespace vulkan */
} /* namespace avis */
