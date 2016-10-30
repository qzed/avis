#include <avis/glfw/vulkan_window.hpp>
#include <avis/vulkan/exception.hpp>


namespace avis {
namespace glfw {

void vulkan_window::create(vulkan::instance& instance, VkAllocationCallbacks const* alloc) {
    if (surface_ != nullptr) return;
    window::create();

    instance_ = &instance;
    alloc_    = alloc;

    auto status = glfwCreateWindowSurface(instance.get_handle(), get_handle(), alloc, &surface_);
    if (status != VK_SUCCESS) {
        destroy();
        throw vulkan::exception(status);
    }
}

void vulkan_window::destroy() noexcept {
    destroy(alloc_);
}

void vulkan_window::destroy(VkAllocationCallbacks const* alloc) noexcept {
    if (surface_ != nullptr) {
        vkDestroySurfaceKHR(instance_->get_handle(), surface_, alloc);
        surface_ = nullptr;
    }

    window::destroy();
}

} /* namespace glfw */
} /* namespace avis */
