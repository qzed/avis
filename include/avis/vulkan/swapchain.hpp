#pragma once

#include <avis/vulkan/handle.hpp>
#include <avis/vulkan/expected.hpp>
#include <avis/vulkan/device.hpp>
#include <avis/vulkan/handles.hpp>
#include <avis/glfw/vulkan_window.hpp>
#include <vector>


namespace avis {
namespace vulkan {

class swapchain {
public:
    swapchain()
            : device_{nullptr}
            , window_{nullptr}
            , format_{}
            , mode_{}
            , extent_{}
            , swapchain_{}
            , images_{}
            , image_views_{} {}

    swapchain(vulkan::device const& device, glfw::vulkan_window const& window, VkSurfaceFormatKHR format,
              VkPresentModeKHR mode, VkExtent2D extent, handle<VkSwapchainKHR>&& swapchain,
              std::vector<VkImage> images, std::vector<handle<VkImageView>>&& image_views)
            : device_{&device}
            , window_{&window}
            , format_{format}
            , mode_{mode}
            , extent_{extent}
            , swapchain_{std::forward<handle<VkSwapchainKHR>>(swapchain)}
            , images_{std::forward<std::vector<VkImage>>(images)}
            , image_views_{std::forward<std::vector<handle<VkImageView>>>(image_views)} {}

    swapchain(swapchain const& other) = delete;
    swapchain(swapchain&& other)      = default;

    auto inline operator= (swapchain const& other) -> swapchain& = delete;
    auto inline operator= (swapchain&&)            -> swapchain& = default;

    auto recreate() noexcept -> vulkan::result;
    inline void destroy() noexcept;

    inline auto get_surface_format() const noexcept -> VkSurfaceFormatKHR const&;
    inline auto get_present_mode()   const noexcept -> VkPresentModeKHR;
    inline auto get_extent()         const noexcept -> VkExtent2D const&;
    inline auto get_swapchain()      const noexcept -> VkSwapchainKHR;
    inline auto get_images()         const noexcept -> std::vector<VkImage> const&;
    inline auto get_image_views()    const noexcept -> std::vector<image_view> const&;

private:
    vulkan::device const*      device_;
    glfw::vulkan_window const* window_;
    VkSurfaceFormatKHR         format_;
    VkPresentModeKHR           mode_;

    VkExtent2D                 extent_;
    handle<VkSwapchainKHR>     swapchain_;
    std::vector<VkImage>       images_;
    std::vector<image_view>    image_views_;
};


auto make_swapchain(device const& device, glfw::vulkan_window const& surface,
        VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<swapchain>;


void swapchain::destroy() noexcept {
    format_ = {};
    mode_   = {};
    extent_ = {};

    image_views_.clear();
    images_.clear();
    swapchain_.destroy();
}

auto swapchain::get_surface_format() const noexcept -> VkSurfaceFormatKHR const& {
    return format_;
}

auto swapchain::get_present_mode() const noexcept -> VkPresentModeKHR {
    return mode_;
}

auto swapchain::get_extent() const noexcept -> VkExtent2D const& {
    return extent_;
}

auto swapchain::get_swapchain() const noexcept -> VkSwapchainKHR {
    return swapchain_.get_handle();
}

auto swapchain::get_images() const noexcept -> std::vector<VkImage> const& {
    return images_;
}

auto swapchain::get_image_views() const noexcept -> std::vector<image_view> const& {
    return image_views_;
}

} /* namespace vulkan */
} /* namespace avis */
