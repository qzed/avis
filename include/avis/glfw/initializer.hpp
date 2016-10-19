#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

namespace avis {
namespace glfw {

class scope_initializer {
public:
  scope_initializer();
  ~scope_initializer();
};

scope_initializer::scope_initializer() { glfwInit(); }

scope_initializer::~scope_initializer() { glfwTerminate(); }

} /* namespace glfw */
} /* namespace avis */
