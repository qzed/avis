#pragma once

#include <avis/vulkan/buffer.hpp>
#include <array>


namespace avis {
namespace vulkan {

class screenquad {
public:
    struct vertex {
        float pos[2];
        float texcoord[2];

        static auto get_binding_description()    -> VkVertexInputBindingDescription;
        static auto get_attribute_descriptions() -> std::array<VkVertexInputAttributeDescription, 2>;
    };

    using index_type = std::uint16_t;

    constexpr static auto vertices = std::array<vertex, 4>{{
        {{-1.0f, -1.0f}, {0.0f, 0.0f}},
        {{-1.0f,  1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 1.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 0.0f}}
    }};

    constexpr static auto indices = std::array<index_type, 4>{{
        1, 2, 0, 3
    }};


public:
    screenquad()
            : buffer_{} {}

    screenquad(buffer&& buf)
            : buffer_{std::forward<buffer>(buf)} {}

    screenquad(screenquad const& other) = delete;
    screenquad(screenquad&& other)      = default;

    inline void destroy() noexcept;

    auto operator= (screenquad const& other) -> screenquad& = delete;
    auto operator= (screenquad&& other)      -> screenquad& = default;

    inline void cmd_draw(VkCommandBuffer buffer) const noexcept;

private:
    buffer buffer_;
};

auto make_screenquad(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue queue)
        noexcept -> vulkan::expected<screenquad>;


void screenquad::destroy() noexcept {
    buffer_.destroy();
}

void screenquad::cmd_draw(VkCommandBuffer buffer) const noexcept {
    auto const buffers = std::array<VkBuffer, 1>{{ buffer_.get_handle() }};
    auto const offsets = std::array<VkDeviceSize, 1>{{ 0 }};
    auto const offset  = sizeof(vulkan::screenquad::vertices[0]) * vulkan::screenquad::vertices.size();

    vkCmdBindVertexBuffers(buffer, 0, buffers.size(), buffers.data(), offsets.data());
    vkCmdBindIndexBuffer(buffer, buffer_.get_handle(), offset, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(buffer, indices.size(), 1, 0, 0, 0);
}

} /* namespace vulkan */
} /* namespace avis */
