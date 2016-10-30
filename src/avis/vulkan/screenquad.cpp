#include <avis/vulkan/screenquad.hpp>

#include <avis/vulkan/command_buffers.hpp>
#include <cstring>


namespace avis {
namespace vulkan {

constexpr decltype(screenquad::vertices) screenquad::vertices;
constexpr decltype(screenquad::indices)  screenquad::indices;

auto screenquad::vertex::get_binding_description() -> VkVertexInputBindingDescription {
    VkVertexInputBindingDescription desc = {};
    desc.binding   = 0;
    desc.stride    = sizeof(vertex);
    desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return desc;
}

auto screenquad::vertex::get_attribute_descriptions() -> std::array<VkVertexInputAttributeDescription, 2> {
    auto desc = std::array<VkVertexInputAttributeDescription, 2>{};

    desc[0].binding  = 0;
    desc[0].location = 0;
    desc[0].format   = VK_FORMAT_R32G32_SFLOAT;
    desc[0].offset   = offsetof(vertex, pos);

    desc[1].binding  = 0;
    desc[1].location = 1;
    desc[1].format   = VK_FORMAT_R32G32_SFLOAT;
    desc[1].offset   = offsetof(vertex, texcoord);

    return desc;
}


auto make_screenquad(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue queue)
        noexcept -> vulkan::expected<screenquad>
{
    VkDeviceSize vertex_buffer_size = sizeof(vulkan::screenquad::vertices[0]) * vulkan::screenquad::vertices.size();
    VkDeviceSize index_buffer_size  = sizeof(vulkan::screenquad::indices[0]) * vulkan::screenquad::indices.size();

    // host-accessible staging buffer
    auto staging_buffer = vulkan::buffer{};
    {
        auto const size  = vertex_buffer_size + index_buffer_size;
        auto const usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        auto const flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        staging_buffer = vulkan::make_exclusive_buffer(device, physical_device, size, usage, flags)
                .move_or_throw();

        // fill vertex buffer
        void* data = staging_buffer.map_memory(device, 0, vertex_buffer_size, 0).move_or_throw();
        std::memcpy(data, vulkan::screenquad::vertices.data(), vertex_buffer_size);
        staging_buffer.unmap_memory(device);

        // fill index buffer
        data = staging_buffer.map_memory(device, vertex_buffer_size, index_buffer_size, 0).move_or_throw();
        std::memcpy(data, vulkan::screenquad::indices.data(), index_buffer_size);
        staging_buffer.unmap_memory(device);
    }

    // device-local vertex buffer
    auto const size  = vertex_buffer_size + index_buffer_size;
    auto const usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                            | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    auto const flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    auto vertex_buffer = make_exclusive_buffer(device, physical_device, size, usage, flags);
    AVIS_VULKAN_EXCEPT_RETURN(vertex_buffer.status());

    // transfer from staging to device-local buffer
    {
        // create command buffer
        auto alloc_info = VkCommandBufferAllocateInfo{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool        = command_pool;
        alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;
        auto cmdbufs = make_command_buffers(device, alloc_info).move_or_throw();
        auto cmdbuf = cmdbufs[0];

        // write copy-command to command-buffer
        auto begin_info = VkCommandBufferBeginInfo{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        AVIS_VULKAN_EXCEPT_RETURN(vkBeginCommandBuffer(cmdbuf, &begin_info));

        auto bufcpy = VkBufferCopy{};
        bufcpy.srcOffset = 0;
        bufcpy.dstOffset = 0;
        bufcpy.size      = vertex_buffer_size + index_buffer_size;

        vkCmdCopyBuffer(cmdbuf, staging_buffer.get_handle(), vertex_buffer.value().get_handle(), 1, &bufcpy);

        AVIS_VULKAN_EXCEPT_RETURN(vkEndCommandBuffer(cmdbuf));

        // submit command buffer
        auto submit_info = VkSubmitInfo{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers    = &cmdbuf;

        AVIS_VULKAN_EXCEPT_RETURN(vkQueueSubmit(queue, 1, &submit_info, nullptr));
        AVIS_VULKAN_EXCEPT_RETURN(vkQueueWaitIdle(queue));
    }

    return { vertex_buffer.move() };
}

} /* namespace vulkan */
} /* namespace avis */
