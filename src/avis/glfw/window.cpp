#include <avis/glfw/window.hpp>
#include <stdexcept>


namespace avis {
namespace glfw {

void window::create() {
    if (window_ != nullptr) return;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, resizable_ ? GLFW_TRUE : GLFW_FALSE);
    window_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
    if (window_ == nullptr)
        throw std::runtime_error("Failed to create GLFW window.");

    try {
        glfwSetWindowUserPointer(window_, this);

        glfwSetKeyCallback(window_, [](GLFWwindow* native, int key, int scancode, int action, int mods) {
            window* self = static_cast<window*>(glfwGetWindowUserPointer(native));

            if (self->cb_input_key_)
                self->cb_input_key_(key, scancode, action, mods);
        });

        glfwSetWindowSizeCallback(window_, [](GLFWwindow* native, int width, int height) {
            window* self = static_cast<window*>(glfwGetWindowUserPointer(native));
            self->width_  = width;
            self->height_ = height;

            if (self->cb_resize_)
                self->cb_resize_(width, height);
        });

    } catch (...) {
        destroy();
        throw;
    }
}

void window::destroy() {
    if (window_ != nullptr) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
}

} /* namespace glfw */
} /* namespace avis */
