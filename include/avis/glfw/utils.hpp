#pragma once

#include <GLFW/glfw3.h>
#include <vector>


namespace avis {
namespace glfw {

template <class Allocator = std::vector<char const*>::allocator_type>
inline auto get_required_instance_extensions(Allocator const& alloc = {}) -> std::vector<char const*, Allocator> {
    uint32_t count = 0;
    char const** extensions = glfwGetRequiredInstanceExtensions(&count);
    return { extensions, extensions + count, alloc};
}

} /* namespace glfw */
} /* namespace avis */
