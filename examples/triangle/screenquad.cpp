#include "screenquad.hpp"


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

} /* namespace vulkan */
} /* namespace avis */
