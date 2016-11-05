#pragma once

#include <avis/glfw/window.hpp>
#include <avis/vulkan/instance.hpp>


namespace avis {
namespace glfw {

class vulkan_window : private glfw::window {
public:
    vulkan_window()
        : window{}
        , instance_{nullptr}
        , alloc_{nullptr}
        , surface_{nullptr} {}

    vulkan_window(std::string title, unsigned int width, unsigned int height, bool resizable = true)
        : window{title, width, height, resizable}
        , instance_{nullptr}
        , alloc_{nullptr}
        , surface_{nullptr} {}

    vulkan_window(vulkan_window&& other)
        : window{std::forward<vulkan_window>(other)}
        , instance_{std::move(other.instance_)}
        , alloc_{std::exchange(other.alloc_, nullptr)}
        , surface_{std::exchange(other.surface_, nullptr)} {}

    vulkan_window(vulkan_window const& other) = delete;

    virtual ~vulkan_window() { destroy(); }

    inline auto operator= (vulkan_window const& rhs) -> vulkan_window& = delete;
    inline auto operator= (vulkan_window&& rhs)      -> vulkan_window&;

    void create(avis::vulkan::instance& instance, VkAllocationCallbacks const* alloc = nullptr);

    void destroy() noexcept;
    void destroy(VkAllocationCallbacks const* alloc) noexcept;

    using window::set_key_input_callback;
    using window::set_resize_callback;

    using window::get_width;
    using window::get_height;
    using window::get_title;
    using window::get_handle;

    using window::get_terminate_request;
    using window::set_terminate_request;

    using window::poll_events;

    inline auto allocator()       noexcept -> VkAllocationCallbacks const* &;
    inline auto allocator() const noexcept -> VkAllocationCallbacks const* const&;

    inline auto get_surface() const noexcept -> VkSurfaceKHR const&;

private:
    avis::vulkan::instance const* instance_;
    VkAllocationCallbacks const*  alloc_;
    VkSurfaceKHR                  surface_;
};

auto vulkan_window::operator= (vulkan_window&& rhs) -> vulkan_window& {
    destroy();
    window::operator=(std::forward<window>(rhs));
    instance_ = std::move(rhs.instance_);
    alloc_    = std::exchange(rhs.alloc_, nullptr);
    surface_  = std::exchange(rhs.surface_, nullptr);
    return *this;
}

auto vulkan_window::allocator() noexcept -> VkAllocationCallbacks const* & {
    return alloc_;
}

auto vulkan_window::allocator() const noexcept -> VkAllocationCallbacks const* const& {
    return alloc_;
}

auto vulkan_window::get_surface() const noexcept -> VkSurfaceKHR const& {
    return surface_;
}

} /* namespace glfw */
} /* namespace avis */
