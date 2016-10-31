#include <avis/vulkan/swapchain.hpp>
#include <avis/utils/algorithm.hpp>
#include <avis/vulkan/enumerations.hpp>


namespace avis {
namespace vulkan {

namespace {

struct swapchain_base {
    VkSurfaceFormatKHR      format;
    VkPresentModeKHR        mode;
    VkExtent2D              extent;
    handle<VkSwapchainKHR>  swapchain;
    std::vector<VkImage>    images;
    std::vector<image_view> image_views;

    swapchain_base() {}

    swapchain_base(VkSurfaceFormatKHR format, VkPresentModeKHR mode, VkExtent2D extent,
                   handle<VkSwapchainKHR>&& swapchain, std::vector<VkImage>&& images,
                   std::vector<handle<VkImageView>>&& image_views)
            : format{format}
            , mode{mode}
            , extent{extent}
            , swapchain{std::forward<handle<VkSwapchainKHR>>(swapchain)}
            , images{std::forward<std::vector<VkImage>>(images)}
            , image_views{std::forward<std::vector<handle<VkImageView>>>(image_views)} {}
};

auto swapchain_recreate_base(device const& device, glfw::vulkan_window const& window, VkSwapchainKHR old,
        VkAllocationCallbacks const* alloc) noexcept -> expected<swapchain_base>
{
    using std::begin;
    using std::end;

    // select surface format
    auto format = VkSurfaceFormatKHR{};
    auto formats = vulkan::get_physical_device_surface_formats_khr(device.get_physical_device(), window.get_surface());
    if (!formats) return formats.status();

    if (formats.value().empty())
        return result::error_incompatible_display_khr;

    if (formats.value().size() == 1 && formats.value()[0].format == VK_FORMAT_UNDEFINED)
        format = {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    auto preferred_format = VkSurfaceFormatKHR{VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    auto is_preferred_format = [&](auto const& fmt){
        return fmt.format == preferred_format.format && fmt.colorSpace == preferred_format.colorSpace;
    };

    if (std::find_if(begin(formats.value()), end(formats.value()), is_preferred_format) != end(formats.value()))
        format = preferred_format;
    else
        format = formats.value()[0];

    // select present mode
    auto mode = VkPresentModeKHR{};
    auto modes = vulkan::get_physical_device_surface_present_modes_khr(device.get_physical_device(), window.get_surface());

    if (modes.value().empty())
        return result::error_incompatible_display_khr;

    if (std::find(begin(modes.value()), end(modes.value()), VK_PRESENT_MODE_MAILBOX_KHR) != end(modes.value()))
        mode = VK_PRESENT_MODE_MAILBOX_KHR;
    else
        mode = VK_PRESENT_MODE_FIFO_KHR;

    // get surface capabilities
    auto caps = VkSurfaceCapabilitiesKHR{};
    AVIS_VULKAN_EXCEPT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.get_physical_device(),
            window.get_surface(), &caps));

    // select extent
    auto extent = caps.currentExtent;
    if (extent.width == 0xFFFFFFFF && extent.height == 0xFFFFFFFF) {
        extent.width = utils::clamp(window.get_width(), caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = utils::clamp(window.get_height(), caps.minImageExtent.height, caps.maxImageExtent.height);
    }

    // select number of swapchain images
    auto image_count = std::max(caps.minImageCount, 2U);
    if (caps.maxImageCount > 0)
        image_count = std::min(caps.maxImageCount, image_count);

    // set up swapchain create-info
    auto swapchain_info = VkSwapchainCreateInfoKHR{};
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.surface          = window.get_surface();
    swapchain_info.minImageCount    = image_count;
    swapchain_info.imageFormat      = format.format;
    swapchain_info.imageColorSpace  = format.colorSpace;
    swapchain_info.imageExtent      = extent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (device.get_graphics_queue().index() != device.get_present_queue().index()) {
        std::uint32_t queue_family_indices[] = {device.get_graphics_queue().index(), device.get_present_queue().index()};
        swapchain_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices   = queue_family_indices;
    } else {
        swapchain_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info.queueFamilyIndexCount = 0;
        swapchain_info.pQueueFamilyIndices   = nullptr;
    }

    swapchain_info.preTransform   = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode    = mode;
    swapchain_info.clipped        = true;
    swapchain_info.oldSwapchain   = old;

    // create swapchain
    auto const device_handle = device.get_handle();

    auto swapchain_handle = VkSwapchainKHR{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateSwapchainKHR(device_handle, &swapchain_info, alloc, &swapchain_handle));

    auto swapchain = make_handle(swapchain_handle, alloc, [=](auto s, auto a){
        vkDestroySwapchainKHR(device_handle, s, a);
    });
    if (!swapchain) return swapchain.status();

    // get swapchain images
    auto images = get_swapchain_images_khr(device_handle, swapchain_handle);
    if (!images) return images.status();

    // set up image views
    auto image_views = std::vector<handle<VkImageView>>{};
    image_views.reserve(images.value().size());

    for (auto image : images.value()) {
        VkImageViewCreateInfo image_view_info{};
        image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_info.image    = image;
        image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_info.format   = format.format;

        image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        image_view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_info.subresourceRange.baseMipLevel   = 0;
        image_view_info.subresourceRange.levelCount     = 1;
        image_view_info.subresourceRange.baseArrayLayer = 0;
        image_view_info.subresourceRange.layerCount     = 1;

        auto image_view_handle = VkImageView{};
        AVIS_VULKAN_EXCEPT_RETURN(vkCreateImageView(device_handle, &image_view_info, alloc, &image_view_handle));

        auto image_view = make_handle(image_view_handle, alloc, [=](auto h, auto a){
            vkDestroyImageView(device_handle, h, a);
        });
        if (!image_view) return image_view.status();

        image_views.push_back(image_view.move());
    }

    return {{format, mode, extent, swapchain.move(), images.move(), std::move(image_views)}};
}

} /* namespace */


auto make_swapchain(device const& device, glfw::vulkan_window const& window, VkAllocationCallbacks const* alloc)
        noexcept -> expected<swapchain>
{
    auto base = swapchain_recreate_base(device, window, nullptr, alloc);
    if (!base) return base.status();

    return {{device, window, base.value().format, base.value().mode, base.value().extent,
            std::move(base.value().swapchain), std::move(base.value().images), std::move(base.value().image_views)}};
}

auto swapchain::recreate() noexcept -> vulkan::result {
    auto base = swapchain_recreate_base(*device_, *window_, swapchain_.get_handle(), swapchain_.allocator());
    if (!base) return base.status();

    image_views_ = std::move(base.value().image_views);
    images_      = std::move(base.value().images);
    swapchain_   = std::move(base.value().swapchain);

    extent_      = base.value().extent;
    format_      = base.value().format;
    mode_        = base.value().mode;

    return result::success;
}

} /* namespace vulkan */
} /* namespace avis */
