#pragma once

#include <avis/vulkan/vulkan.hpp>
#include <avis/vulkan/expected.hpp>
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
};

inline auto make_screenquad(VkDevice device) -> vulkan::expected<screenquad>;


} /* namespace vulkan */
} /* namespace avis */
